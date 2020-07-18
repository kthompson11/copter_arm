#include "stm32f7xx.h"
#include "system_stm32f7xx.h"

int main(void)
{
    /* enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* set green led pin as output */
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_0);
    
    /* turn on green led */
    GPIOB->ODR |= GPIO_ODR_ODR_0;

    return 0;
}