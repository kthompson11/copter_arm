#include <stm32f7xx.h>
#include <system_stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"
#include "copter.h"

void blink_task(void *param)
{
    for (;;) {
        GPIOB->ODR ^= GPIO_ODR_ODR_0;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    static struct usart_task_param usart3_param;
    usart3_param.tx_pool_size = 3;
    usart3_param.tx_pool = xQueueCreate(3, sizeof(struct usart_buffer *));
    usart3_param.tx_queue = xQueueCreate(3, sizeof(struct usart_buffer *));
    usart3_param.tx_done = xSemaphoreCreateBinary();

    static struct copter_task_param copter_param;
    copter_param.usart3_tx_queue = usart3_param.tx_queue;
    copter_param.usart3_tx_pool = usart3_param.tx_pool;

    /* enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* set green led pin as output */
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_0);
    
    /* turn on green led */
    GPIOB->ODR |= GPIO_ODR_ODR_0;

    xTaskCreate(blink_task, "blink", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(copter_task, "copter", 512, &copter_param, 2, NULL);
    xTaskCreate(usart_task, "usart", 512, &usart3_param, 3, NULL);
    
    vTaskStartScheduler();

    return 0;
}