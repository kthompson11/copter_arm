#include <stm32f7xx.h>
#include <system_stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

void blink_task(void *param)
{
    for (;;) {
        GPIOB->ODR ^= GPIO_ODR_ODR_0;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    /* enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* set green led pin as output */
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_0);
    
    /* turn on green led */
    GPIOB->ODR |= GPIO_ODR_ODR_0;

    xTaskCreate(blink_task, "blink", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    return 0;
}