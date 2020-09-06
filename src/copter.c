#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "copter.h"
#include "pid_control.h"

static uint16_t LastADCValue;
static const int OUTPUT_PERIOD = 10;
int output_count = 0;

void copter_task(void *_param)
{
    struct copter_task_param *param = (struct copter_task_param *)_param;

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

    const float KI = 0.1;
    struct pid_state state = {
        .KP = 0.2,
        .KI = KI,
        .KD = 0.2,
        .T = 0.01,
        .MAXSUM = 0.3 * TIMER_MAX_COUNT / KI,
        .MINSUM = -0.3 * TIMER_MAX_COUNT / KI,
        .DERIV_INTERVAL = 1,
    };
    int32_t x_d = 2300;

    for (;;) {
        /* get arm angle value */
        ADC1->CR2 |= ADC_CR2_ADON;

        uint32_t adc_value = 0;
        const int N_ADC_TERMS = 10;
        for (int i = 0; i < N_ADC_TERMS; ++i) {
            ADC1->CR2 |= ADC_CR2_SWSTART;
            while (!(ADC1->SR & ADC_SR_EOC)) {}  /* wait for end of conversion */
            adc_value += ADC1->DR;
        }
        
        adc_value /= N_ADC_TERMS;
        LastADCValue = adc_value;

        /* calculate error */
        int32_t x = adc_value;
        float error = x_d - x;

        /* get next controller output */
        float output = pid_tick(&state, error);
        if (output > TIMER_MAX_COUNT) {
            output = TIMER_MAX_COUNT;
        } else if (output < 0) {
            output = 0;
        }
        uint32_t c = output;

        /* set motor pwm */
        TIM5->CCR1 = c;

        /* output LastADCValue to serial port for testing */
        if (output_count == 0) {
            uint8_t buf[2];
            unsigned int len = 2;
            buf[0] = LastADCValue >> 8;
            buf[1] = LastADCValue >> 0;
            usart_write(param->usart3, buf, len, 0);
        }
        output_count = (output_count + 1) % OUTPUT_PERIOD;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}