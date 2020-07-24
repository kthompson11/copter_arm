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

    /* setup GPIO pins for TIM3 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER0, 0b10 << GPIO_MODER_MODER0_Pos);
    MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL0, 2 << GPIO_AFRL_AFRL0_Pos);

    /* setup ADC1 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    // ADC1->CR1 |= ADC_CR1_EOCIE;  /* enable end of conversion interrupt */
    MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1, 9 << ADC_SQR3_SQ1_Pos);  /* select input 9 for first conversion */
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP0, 0b001 << ADC_SMPR2_SMP0_Pos);

    /* setup TIM3 */
    const uint32_t PWM_Frequency = 25000;  /* 25kHz */
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    int presc_index = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    int presc = APBPrescTable[presc_index];
    uint32_t apb1_frequency = SystemCoreClock >> presc;
    if (presc > 0) {
        apb1_frequency = apb1_frequency << 1;
    }
    uint32_t timer_max_count = apb1_frequency / PWM_Frequency;
    MODIFY_REG(TIM5->CCMR1, TIM_CCMR1_OC1M, 0x6 << TIM_CCMR1_OC1M_Pos);  /* set PWM mode 1 */
    TIM5->ARR = timer_max_count;
    TIM5->CCER |= TIM_CCER_CC1E;
    TIM5->CR1 |= TIM_CR1_CEN;

    for (;;) {
        /* get arm angle value */
        ADC1->CR2 |= ADC_CR2_ADON;
        ADC1->CR2 |= ADC_CR2_SWSTART;
        while (!(ADC1->SR & ADC_SR_EOC)) {}  /* wait for end of conversion */
        uint32_t value = ADC1->DR;
        LastADCValue = value;

        /* set motor pwm */
        /* set 20% duty cycle */
        uint32_t count = TIM5->CNT;
        TIM5->CCR1 = timer_max_count * 20 / 100;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}