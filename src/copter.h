/* Provides the copter motor control task */

#ifndef COPTER_H
#define COPTER_H

#include <queue.h>

#include "usart.h"

struct copter_task_param {
    const struct usart *usart3;
};

void copter_task(void *param);

#endif /* COPTER_H */