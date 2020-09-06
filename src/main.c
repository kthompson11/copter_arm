#include <stm32f7xx.h>
#include <system_stm32f7xx.h>

#include <FreeRTOS.h>
#include <task.h>

#include "usart.h"
#include "copter.h"

/********************************* seperate usart3 file *********************************/

static struct usart usart3;

const struct usart* usart3_setup(void)
{
    /* usart_init(usart3_obj, ...)  creates all queues/semaphores */
    usart_init(&usart3, 0, 3, 0, 256, 9600, USART3, DMA1_Stream3);

    /* setup clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;  /* enable DMA1 clock */
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /* setup USART port */
    /* use GPIOD port 8 */
    MODIFY_REG(GPIOD->MODER, GPIO_MODER_MODER8, GPIO_MODER_MODER8_1);
    MODIFY_REG(GPIOD->AFR[1], GPIO_AFRH_AFRH0, 0x7UL << GPIO_AFRH_AFRH0_Pos);

    /* setup DMA */
    CLEAR_BIT(DMA1_Stream3->CR, DMA_SxCR_EN);
    while (DMA1_Stream3->CR & DMA_SxCR_EN) {}  /* busy wait for EN to go to zero */
    DMA1_Stream3->PAR = (volatile uint32_t)&USART3->TDR;
    DMA1_Stream3->CR |= 4 << DMA_SxCR_CHSEL_Pos;
    DMA1_Stream3->CR |= 1 << DMA_SxCR_MINC_Pos;
    DMA1_Stream3->CR |= 0b01 << DMA_SxCR_DIR_Pos;

    /* enable interrupts */
    NVIC_EnableIRQ(DMA1_Stream3_IRQn);

    /* create task */
    xTaskCreate(usart_task, "usart", 512, &usart3, 3, NULL);

    return &usart3;
}

void DMA1_Stream3_IRQHandler(void)
{
    BaseType_t context_switch = pdFALSE;

    if (DMA1->LISR & DMA_LISR_TCIF3) {
        /* send tx done signal */
        xSemaphoreGiveFromISR(usart3.tx_done, &context_switch);
    }

    DMA1->LIFCR = 0x3DUL << DMA_LIFCR_CFEIF3_Pos;  /* clear all interrupt flags for stream 3 */

    portYIELD_FROM_ISR(context_switch);
}

/********************************* end seperate usart3 file *********************************/

void blink_task(void *param)
{
    for (;;) {
        GPIOB->ODR ^= GPIO_ODR_ODR_0;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    const struct usart *usart3 = usart3_setup();;

    static struct copter_task_param copter_param;
    copter_param.usart3 = usart3;

    /* enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /* set green led pin as output */
    MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODER0, GPIO_MODER_MODER0_0);
    
    /* turn on green led */
    GPIOB->ODR |= GPIO_ODR_ODR_0;

    xTaskCreate(blink_task, "blink", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(copter_task, "copter", 512, &copter_param, 2, NULL);
    
    vTaskStartScheduler();

    return 0;
}