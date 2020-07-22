#include <string.h>

#include <stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"
#include "copter.h"

/**
 * Sets the baud rate assuming oversampling by 16 and SYSCLK is used as clock source
 */
void usart_set_baud_rate(unsigned long rate_hz)
{
    int prescaler_value = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    uint32_t sysclk_frequency = SystemCoreClock << prescaler_value;
    USART3->BRR = sysclk_frequency / rate_hz;
}

int sprint_uint(char *buf, uint32_t n)
{
    if (n == 0) {
        *buf = '0';
        return 1;
    }

    const char characters[] = "0123456789";
    int j = 0;
    int bytes_written;
    uint32_t remaining_n = n;

    while (remaining_n > 0) {
        int digit = remaining_n % 10;
        remaining_n /= 10;
        
        buf[j] = characters[digit];
        j += 1;
    }
    bytes_written = j;

    /* reverse string */
    int i;
    for (i = 0, --j; i <= j; ++i, --j) {
        char tmp;
        tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
    }

    return bytes_written;
}

int sprint_int(char *buf, int32_t n)
{
    if (n < 0) {
        *buf = '-';
        return sprint_uint(buf + 1, -n) + 1;
    } else {
        return sprint_uint(buf, n);
    }
}

void usart_task(void *param)
{
    char message[100];
    //const char *message = "0";

    /* setup DMA */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  /* enable DMA1 clock */
    CLEAR_BIT(DMA1_Stream3->CR, DMA_SxCR_EN);
    while (DMA1_Stream3->CR & DMA_SxCR_EN) {}  /* busy wait for EN to go to zero */
    DMA1_Stream3->PAR = (volatile uint32_t)&USART3->TDR;
    DMA1_Stream3->CR |= 4 << DMA_SxCR_CHSEL_Pos;
    DMA1_Stream3->CR |= 1 << DMA_SxCR_MINC_Pos;
    DMA1_Stream3->CR |= 0b01 << DMA_SxCR_DIR_Pos;

    /* setup GPIOD for USART */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    MODIFY_REG(GPIOD->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_1);
    MODIFY_REG(GPIOD->AFR[1], GPIO_AFRH_AFRH0, 0x7UL << GPIO_AFRH_AFRH0_Pos);

    /* setup USART3 */
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    usart_set_baud_rate(9600);
    USART3->CR1 |= USART_CR1_UE;
    USART3->CR3 |= USART_CR3_DMAT;
    USART3->CR1 |= USART_CR1_TE;

    for (;;) {
        while (DMA1_Stream3->CR & DMA_SxCR_EN) {}  /* wait until DMA is done */
        int n_bytes = sprint_uint(message, LastADCValue);
        message[n_bytes] = '\n';
        message[n_bytes+1] = '\0';
        DMA1_Stream3->M0AR = (volatile uint32_t)message;  /* reset source address */
        DMA1_Stream3->NDTR = strlen(message);  /* reset data length */
        USART3->ICR = USART_ICR_TCCF;
        DMA1->LIFCR = 0x3DUL << DMA_LIFCR_CFEIF3_Pos;  /* clear all interrupt flags for stream 3 */
        DMA1_Stream3->CR |= DMA_SxCR_EN;

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}