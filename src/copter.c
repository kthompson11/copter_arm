#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "copter.h"

uint16_t LastADCValue;

void copter_task(void *param)
{
    /* setup GPIO pins for ADC1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER1, 0b11 << GPIO_MODER_MODER1_Pos);  /* set analog mode */

    /* setup ADC1 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    // ADC1->CR1 |= ADC_CR1_EOCIE;  /* enable end of conversion interrupt */
    MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1, 9 << ADC_SQR3_SQ1_Pos);  /* select input 9 for first conversion */
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP0, 0b001 << ADC_SMPR2_SMP0_Pos);

    for (;;) {
        ADC1->CR2 |= ADC_CR2_ADON;
        ADC1->CR2 |= ADC_CR2_SWSTART;
        while (!(ADC1->SR & ADC_SR_EOC)) {}  /* wait for end of conversion */
        uint32_t value = ADC1->DR;
        LastADCValue = value;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}