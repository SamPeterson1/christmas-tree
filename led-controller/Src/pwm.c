#include "pwm.h"

/* Initialize PWM with DMA on stream 5 channel 6 */
void init_pwm(uint16_t *dma_buf, uint16_t size) {
    /* Configure GPIOs */
    gpio_set_mode(GPIO_PORT_E, GPIO_PIN_9, GPIO_MODE_AF);
	gpio_set_af(GPIO_PORT_E, GPIO_PIN_9, GPIO_AF1);
	gpio_set_ospeed(GPIO_PORT_E, GPIO_PIN_9, GPIO_OSPEED_VHIGH);

    DMA2_Stream5->PAR = (uint32_t) &TIM1->DMAR; /* Set peripheral register address */
    DMA2_Stream5->M0AR = dma_buf; /* Set memory address of DMA buffer */
    DMA2_Stream5->NDTR = size; /* Set number of data items to transfer (N_LEDS * 24 bits per led) */

    DMA2_Stream5->CR |= (0b110 << 25); /* Select DMA channel 6 */
    DMA2_Stream5->CR |= (0b01 << 6); /* Set direction to memory-to-peripheral */
    DMA2_Stream5->CR |= (0b01 << 11); /* Set peripheral data size to 16 bits */
    DMA2_Stream5->CR |= (0b01 << 13); /* Set memory data size to 16 bits */
    DMA2_Stream5->CR |= DMA_SxCR_MINC; /* Enable memory increment */
    DMA2_Stream5->CR |= DMA_SxCR_CIRC; /* Enable circular mode */
    
    DMA2_Stream5->CR |= DMA_SxCR_EN; /* Enable DMA stream */

    TIM1->PSC = 0; /* No clock prescaler */
    TIM1->ARR = 25; /* Set PWM period to 25 ticks */

    TIM1->DCR |= 13; /* Set DMA target to TIMx_CCR1 */
    TIM1->DIER |= TIM_DIER_UDE; /* Enable DMA request on update */
    TIM1->CCMR1 |= (0b110 << 4); /* Set PWM mode 1 */
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE; /* Enable preload register */
    TIM1->CCER |= TIM_CCER_CC1E;  /* Enable output */
    TIM1->CR1 |= TIM_CR1_ARPE;        /* Enable auto-reload preload register */
    TIM1->CCR1 = 0; /* Zero PWM signal */
    TIM1->BDTR |= TIM_BDTR_MOE; /* Enable main output */

    TIM1->CR1 |= 1; // Start the counter
}