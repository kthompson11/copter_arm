/* Provides the copter motor control task */

#ifndef COPTER_H
#define COPTER_H

#include <queue.h>

#include "usart.h"

struct copter_task_param {
    QueueHandle_t usart3_tx_queue;
    QueueHandle_t usart3_tx_pool;
};

void copter_task(void *param);

#endif /* COPTER_H */