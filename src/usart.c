#include <string.h>

#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"

/**
 * Sets the baud rate assuming oversampling by 16 and SYSCLK is used as clock source
 */
void usart_set_baud_rate(unsigned long rate_hz)
{
    int prescaler_value = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    uint32_t sysclk_frequency = SystemCoreClock << prescaler_value;
    USART3->BRR = sysclk_frequency / rate_hz;
}

void usart_task(void *param)
{
    const char *message = "Hello World!\n";

    /* setup GPIOD for USART */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    MODIFY_REG(GPIOD->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_1);
    MODIFY_REG(GPIOD->AFR[1], GPIO_AFRH_AFRH0, 0x7UL << GPIO_AFRH_AFRH0_Pos);

    /* init UART3 + DMA */
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    usart_set_baud_rate(9600);
    USART3->CR1 |= USART_CR1_UE;
    USART3->CR1 |= USART_CR1_TE;

    /* send byte fore baud rate autodetection */
    USART3->TDR = 0xAA;

    for (;;) {
        /* TODO: send message using DMA */
        int tx_bytes = strlen(message);
        int bytes_transmitted = 0;
        while (bytes_transmitted < tx_bytes) {
            while (!(USART3->ISR & USART_ISR_TXE)) {
                // busy wait for space in tx buffer
            }

            USART3->TDR = message[bytes_transmitted];
            bytes_transmitted += 1;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}