#include <string.h>

#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"
#include "crc32.h"
#include "cobs.h"

/* TODO: rework so that multiple instantiations of usart can be used at a time (no globals, reentrant, choice of crc/framing, etc. */
static SemaphoreHandle_t tx_done;

void DMA1_Stream3_IRQHandler(void)
{
    BaseType_t context_switch = pdFALSE;

    if (DMA1->LISR & DMA_LISR_TCIF3) {
        /* send tx done signal */
        xSemaphoreGiveFromISR(tx_done, &context_switch);
    }

    DMA1->LIFCR = 0x3DUL << DMA_LIFCR_CFEIF3_Pos;  /* clear all interrupt flags for stream 3 */

    portYIELD_FROM_ISR(context_switch);
}

/**
 * Sets the baud rate assuming oversampling by 16 and SYSCLK is used as clock source
 */
void usart_set_baud_rate(unsigned long rate_hz)
{
    int prescaler_value = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    uint32_t sysclk_frequency = SystemCoreClock << prescaler_value;
    USART3->BRR = sysclk_frequency / rate_hz;
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
    struct usart_task_param *param = (struct usart_task_param *)_param;

    /* set semaphore used when a tx finishes */
    tx_done = param->tx_done;

    /* create tx buffers and fill up tx pool */
    struct usart_buffer *tx_buffers = pvPortMalloc(param->tx_pool_size * sizeof(struct usart_buffer));
    for (int i = 0; i < param->tx_pool_size; ++i) {
        /* should always succeed if pool queues are properly sized */
        struct usart_buffer *next_buffer = tx_buffers + i;
        xQueueSend(param->tx_pool, &next_buffer, 0);
    }

    #ifdef COMMENTED_OUT
    /* create rx buffers and fill up rx pool */
    struct usart_buffer *rx_buffers = pvPortMalloc(param->rx_pool_size * sizeof(struct usart_buffer));
    for (int i = 0; i < param->rx_pool_size; ++i) {
        /* should always succeed if pool queues are properly sized */
        struct usart_buffer *next_buffer = rx_buffers + i;
        xQueueSend(param->rx_pool, &next_buffer + i, 0);
    }
    #endif

    /* setup DMA */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  /* enable DMA1 clock */
    CLEAR_BIT(DMA1_Stream3->CR, DMA_SxCR_EN);
    while (DMA1_Stream3->CR & DMA_SxCR_EN) {}  /* busy wait for EN to go to zero */
    DMA1_Stream3->PAR = (volatile uint32_t)&USART3->TDR;
    DMA1_Stream3->CR |= 4 << DMA_SxCR_CHSEL_Pos;
    DMA1_Stream3->CR |= 1 << DMA_SxCR_MINC_Pos;
    DMA1_Stream3->CR |= 0b01 << DMA_SxCR_DIR_Pos;

    /* setup GPIOD for USART */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    MODIFY_REG(GPIOD->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_1);
    MODIFY_REG(GPIOD->AFR[1], GPIO_AFRH_AFRH0, 0x7UL << GPIO_AFRH_AFRH0_Pos);

    /* setup USART3 */
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    usart_set_baud_rate(9600);
    USART3->CR1 |= USART_CR1_UE;
    USART3->CR3 |= USART_CR3_DMAT;
    USART3->CR1 |= USART_CR1_TE;

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
        DMA1_Stream3->M0AR = (uint32_t)tmp_buffer;
        DMA1_Stream3->NDTR = (uint32_t)bs_len;
        DMA1_Stream3->CR |= DMA_SxCR_TCIE;
        NVIC_EnableIRQ(DMA1_Stream3_IRQn);
        USART3->ICR = USART_ICR_TCCF;
        
        xSemaphoreTake(param->tx_done, 0);  /* reset tx_done */
        DMA1_Stream3->CR |= DMA_SxCR_EN;  /* start tx */
        /* wait for tx done signal */
        xSemaphoreTake(tx_done, portMAX_DELAY);
    }
}