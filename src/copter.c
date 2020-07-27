#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "copter.h"

uint16_t LastADCValue;

void copter_task(void *param)
{
    /* setup GPIO pins for ADC1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODER2, 0b11 << GPIO_MODER_MODER2_Pos);  /* set analog mode */

    /* setup GPIO pins for TIM3 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER0, 0b10 << GPIO_MODER_MODER0_Pos);
    MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL0, 2 << GPIO_AFRL_AFRL0_Pos);

    /* setup ADC1 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1, 12 << ADC_SQR3_SQ1_Pos);  /* select input 12 for first conversion */
    MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP0, 0b001 << ADC_SMPR2_SMP0_Pos);

    /* setup TIM3 */
    const uint32_t PWM_FREQUENCY = 25000;  /* 25kHz */
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    int presc_index = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    int presc = APBPrescTable[presc_index];
    uint32_t apb1_frequency = SystemCoreClock >> presc;
    if (presc > 0) {
        apb1_frequency = apb1_frequency << 1;
    }
    const uint32_t TIMER_MAX_COUNT = apb1_frequency / PWM_FREQUENCY;
    MODIFY_REG(TIM5->CCMR1, TIM_CCMR1_OC1M, 0x6 << TIM_CCMR1_OC1M_Pos);  /* set PWM mode 1 */
    TIM5->ARR = TIMER_MAX_COUNT;
    TIM5->CCER |= TIM_CCER_CC1E;
    TIM5->CR1 |= TIM_CR1_CEN;

    /* controller variables and constants*/
    int32_t T_ms = 10;
    int32_t f_controller = 1000 / T_ms;
    int32_t c_direct = TIMER_MAX_COUNT * 17 / 100;
    /* proportional control constants */
    int32_t kp_mul = 1;
    int32_t kp_div = 4;
    /* integral control contstants */
    int32_t ki_mul = 1;
    int32_t ki_div = 80;
    const int32_t MAXSUM = ki_div * f_controller * TIMER_MAX_COUNT / ki_mul / 6;
    const int32_t MINSUM = -MAXSUM;
    /* derivative control constants */
    int32_t kd_mul = 1;
    int32_t kd_div = 2;
    /* controller input and state */
    int32_t x_d = 2400;
    int32_t sum = 0;
    int32_t x = 0;
    int32_t x_last = 0;

    for (;;) {
        /* get arm angle value */
        ADC1->CR2 |= ADC_CR2_ADON;

        uint32_t adc_value = 0;
        const int N_ADC_TERMS = 5;
        for (int i = 0; i < N_ADC_TERMS; ++i) {
            ADC1->CR2 |= ADC_CR2_SWSTART;
            while (!(ADC1->SR & ADC_SR_EOC)) {}  /* wait for end of conversion */
            adc_value += ADC1->DR;
        }
        
        adc_value /= N_ADC_TERMS;
        LastADCValue = adc_value;

        x_last = x;
        x = adc_value;

        /* calculate controller output */
        int32_t error = x_d - x;
        sum = sum + error;
        if (sum > MAXSUM) {
            sum = MAXSUM;
        } else if (sum < MINSUM) {
            sum = MINSUM;
        }

        int32_t c_i = sum * ki_mul / ki_div / f_controller;
        int32_t c_p = error * kp_mul / kp_div;
        int32_t c_d = (x_last - x) * f_controller * kd_mul / kd_div;

        int32_t c = c_i + c_p + c_d + c_direct;
        if (c < 0) {
            c = 0;
        } else if (c > TIMER_MAX_COUNT) {
            c = TIMER_MAX_COUNT;
        }

        /* set motor pwm */
        TIM5->CCR1 = c;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}