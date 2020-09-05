/* Provides a task for setting up the virtual COM port using USART3 */

#ifndef USART_H
#define USART_H

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#define USART_BUFFER_SIZE 256

struct usart_buffer {
    uint8_t data[USART_BUFFER_SIZE];
    int len;
};

struct usart_task_param {
    QueueHandle_t rx_pool;
    QueueHandle_t tx_pool;
    QueueHandle_t rx_queue;
    QueueHandle_t tx_queue;
    int tx_pool_size; /* size of tx pool in number of buffers */
    int rx_pool_size; /* size of rx pool in number of buffers */
    SemaphoreHandle_t tx_done;
};

void usart_task(void *param);

#endif /* USART_H */