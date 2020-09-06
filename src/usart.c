#include <string.h>

#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"
#include "crc32.h"
#include "cobs.h"

void usart_init(struct usart *obj, unsigned int rx_pool_size, unsigned int tx_pool_size, 
                unsigned int rx_buf_size, unsigned int tx_buf_size, uint32_t baud_rate, 
                USART_TypeDef *usart_base, DMA_Stream_TypeDef *dma_stream_base)
{
    obj->rx_pool_size = rx_pool_size;
    obj->tx_pool_size = tx_pool_size;
    obj->rx_buf_size = rx_buf_size;
    obj->tx_buf_size = tx_buf_size;
    obj->baud_rate = baud_rate;
    obj->usart_base = usart_base;
    obj->dma_stream_base = dma_stream_base;

    /* create tx_done semaphore if needed */
    if (dma_stream_base) {
        obj->tx_done = xSemaphoreCreateBinary();
    }

    /* create tx queue and pool */
    if (tx_pool_size > 0) {
        obj->tx_queue = xQueueCreate(tx_pool_size, sizeof(struct usart *));
        obj->tx_pool = xQueueCreate(tx_pool_size, sizeof(struct usart *));
    }

    /* create rx queue and pool */
    if (rx_pool_size > 0) {
        obj->rx_queue = xQueueCreate(rx_pool_size, sizeof(struct usart *));
        obj->rx_pool = xQueueCreate(rx_pool_size, sizeof(struct usart *));
    }

    /* create tx buffers and fill up tx pool */
    struct usart_buffer *tx_buffers = pvPortMalloc(obj->tx_pool_size * sizeof(struct usart_buffer));
    for (int i = 0; i < obj->tx_pool_size; ++i) {
        /* should always succeed if pool queues are properly sized */
        struct usart_buffer *next_buffer = tx_buffers + i;
        next_buffer->data = pvPortMalloc(obj->tx_buf_size);
        next_buffer->max_len = obj->tx_buf_size;
        xQueueSend(obj->tx_pool, &next_buffer, 0);
    }

    /* create rx buffers and fill up rx pool */
    struct usart_buffer *rx_buffers = pvPortMalloc(obj->rx_pool_size * sizeof(struct usart_buffer));
    for (int i = 0; i < obj->rx_pool_size; ++i) {
        /* should always succeed if pool queues are properly sized */
        struct usart_buffer *next_buffer = rx_buffers + i;
        next_buffer->data = pvPortMalloc(obj->rx_buf_size);
        next_buffer->max_len = obj->rx_buf_size;
        xQueueSend(obj->rx_pool, &next_buffer, 0);
    }
}

int usart_read(const struct usart *obj, uint8_t *buf, TickType_t timeout)
{
    struct usart_buffer *usart_buf;

    /* get usart_buffer from queue */
    if (xQueueReceive(obj->rx_queue, &usart_buf, timeout) == errQUEUE_EMPTY) {
        return -1;
    }

    /* copy data */
    int len = usart_buf->len;
    memcpy(buf, usart_buf->data, usart_buf->len);

    /* return usart buffer to rx pool (should always succeed) */
    xQueueSend(obj->rx_pool, &usart_buf, 0);

    return len;
}

int usart_write(const struct usart *obj, const uint8_t *buf, unsigned int len, TickType_t timeout)
{
    struct usart_buffer *usart_buf;

    /* get a usart_buffer from tx pool */
    if (xQueueReceive(obj->tx_pool, &usart_buf, timeout) == errQUEUE_EMPTY) {
        return -1;
    }

    /* copy data */
    int cpy_len;
    if (len > usart_buf->max_len) {
        cpy_len = usart_buf->max_len;
    } else {
        cpy_len = len;
    }
    memcpy(usart_buf->data, buf, cpy_len);
    usart_buf->len = cpy_len;

    /* put usart buffer on the tx queue (should always succeed) */
    xQueueSend(obj->tx_queue, &usart_buf, 0);

    return cpy_len;
}

/**
 * Sets the baud rate assuming oversampling by 16 and SYSCLK is used as clock source
 */
void usart_set_baud_rate(struct usart* usart_periph, unsigned long rate_hz)
{
    int prescaler_value = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    uint32_t sysclk_frequency = SystemCoreClock << prescaler_value;
    usart_periph->usart_base->BRR = sysclk_frequency / rate_hz;
}

int sprint_uint(char *buf, uint32_t n)
{
    if (n == 0) {
        *buf = '0';
        return 1;
    }

    const char characters[] = "0123456789";
    int j = 0;
    int bytes_written;
    uint32_t remaining_n = n;

    while (remaining_n > 0) {
        int digit = remaining_n % 10;
        remaining_n /= 10;
        
        buf[j] = characters[digit];
        j += 1;
    }
    bytes_written = j;

    /* reverse string */
    int i;
    for (i = 0, --j; i <= j; ++i, --j) {
        char tmp;
        tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
    }

    return bytes_written;
}

int sprint_int(char *buf, int32_t n)
{
    if (n < 0) {
        *buf = '-';
        return sprint_uint(buf + 1, -n) + 1;
    } else {
        return sprint_uint(buf, n);
    }
}

void usart_task(void *_param)
{
    struct usart *param = (struct usart *)_param;

    /* setup USART3 */
    usart_set_baud_rate(param, 9600);
    param->usart_base->CR1 |= USART_CR1_UE;
    param->usart_base->CR3 |= USART_CR3_DMAT;
    param->usart_base->CR1 |= USART_CR1_TE;

    struct usart_buffer *tx_buffer;
    uint8_t tmp_buffer[USART_BUFFER_SIZE]; /* for calculating crc32 and cobs */

    for (;;) {
        xQueueReceive(param->tx_queue, &tx_buffer, portMAX_DELAY);
        /* get CRC32 */
        uint32_t crc = crc32(tx_buffer->data, tx_buffer->len);
        /* copy crc to buffer MSB first */
        tx_buffer->data[tx_buffer->len] = crc >> 24;
        tx_buffer->data[tx_buffer->len + 1] = crc >> 16;
        tx_buffer->data[tx_buffer->len + 2] = crc >> 8;
        tx_buffer->data[tx_buffer->len + 3] = crc;
        tx_buffer->len += 4;
        /* byte stuff packet */
        int bs_len = cobs_stuff(tx_buffer->data, tmp_buffer, tx_buffer->len);
        /* return tx_buffer to tx buffer pool */
        xQueueSend(param->tx_pool, &tx_buffer, portMAX_DELAY);

        /* send buffer using DMA */
        param->dma_stream_base->M0AR = (uint32_t)tmp_buffer;
        param->dma_stream_base->NDTR = (uint32_t)bs_len;
        param->dma_stream_base->CR |= DMA_SxCR_TCIE;
        param->usart_base->ICR = USART_ICR_TCCF;
        
        xSemaphoreTake(param->tx_done, 0);  /* reset tx_done */
        param->dma_stream_base->CR |= DMA_SxCR_EN;  /* start tx */
        /* wait for tx done signal */
        xSemaphoreTake(param->tx_done, portMAX_DELAY);
    }
}