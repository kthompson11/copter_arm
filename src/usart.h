/* Provides a task for setting up the virtual COM port using USART3 */

#ifndef USART_H
#define USART_H

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#define USART_BUFFER_SIZE 256

struct usart_buffer {
    uint8_t *data;
    unsigned int len;
    unsigned int max_len;
};

struct usart {
    QueueHandle_t rx_pool;
    QueueHandle_t tx_pool;
    QueueHandle_t rx_queue;
    QueueHandle_t tx_queue;
    int rx_buf_size; /* size of the rx buffer */
    int tx_buf_size;  /* size of the tx buffer */
    int rx_pool_size; /* size of rx pool in number of buffers */
    int tx_pool_size; /* size of tx pool in number of buffers */
    SemaphoreHandle_t tx_done;
    USART_TypeDef *usart_base;
    DMA_Stream_TypeDef *dma_stream_base;
    uint32_t baud_rate;
};

/**
 * Initializes a usart peripheral
 * @param obj pointer to struct representing the USART peripheral
 * @param rx_pool_size the size of the rx pool (0 if disabled)
 * @param tx_pool_size the size of the tx pool (0 if disabled)
 * @param rx_buf_size the size (in bytes) of the tx buffer
 * @param tx_buf_size the size (in bytes) of the tx buffer
 * @param baud_rate the baud rate of the peripheral
 * @param usart_base base address of the usart peripheral
 * @param dma_stream_base base address of the dma stream to use for the peripheral; if NULL, use polling instead
 */
void usart_init(struct usart *obj, unsigned int rx_pool_size, unsigned int tx_pool_size, 
                unsigned int rx_buf_size, unsigned int tx_buf_size, uint32_t baud_rate, 
                USART_TypeDef *usart_base, DMA_Stream_TypeDef *dma_stream_base);

/**
 * Reads from a usart peripheral
 * @param obj pointer to struct representing the USART peripheral
 * @param buf buffer to copy read data to; must be at least obj->rx_buf_len bytes long
 * @param timeout maximum number of ticks to block for; portMAX_DELAY to block indefinitely
 * 
 * @return number of bytes read; -1 on timeout
 */
int usart_read(const struct usart *obj, uint8_t *buf, TickType_t timeout);

/**
 * Reads from a usart peripheral
 * @param obj pointer to struct representing the USART peripheral
 * @param buf buffer to copy written data from
 * @param len number of bytes to copy
 * @param timeout maximum number of ticks to block for; portMAX_DELAY to block indefinitely
 * 
 * @return number of bytes written; -1 on timeout
 */
int usart_write(const struct usart *obj, const uint8_t *buf, unsigned int len, TickType_t timeout);

void usart_task(void *param);

#endif /* USART_H */