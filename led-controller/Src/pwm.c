#include "pwm.h"

void init_pwm(uint16_t *dma_buf, uint16_t size) {
    gpio_set_mode(GPIO_PORT_E, GPIO_PIN_9, GPIO_MODE_AF);
	gpio_set_af(GPIO_PORT_E, GPIO_PIN_9, GPIO_AF1);
	gpio_set_ospeed(GPIO_PORT_E, GPIO_PIN_9, GPIO_OSPEED_VHIGH);

    //Set peripheral port register addr in DMA_SxPAR
    DMA2_Stream5->PAR = (uint32_t) &TIM1->DMAR;
    //Set memory address in DMA_SxMA0R register
    DMA2_Stream5->M0AR = dma_buf;
    //Set number of data items to transfer in DMA_SxNDTR register
    DMA2_Stream5->NDTR = size;
    //Select DMA channel using DMA_SxCR register
    DMA2_Stream5->CR |= (0b110 << 25); //Channel 6 select
    DMA2_Stream5->CR |= (0b01 << 6); //Memory-to-peripheral
    DMA2_Stream5->CR |= (0b01 << 11); //Set peripheral data size to 16 bits
    DMA2_Stream5->CR |= (0b01 << 13); //Set memory data size to 16 bits
    DMA2_Stream5->CR |= (1 << 10); //Enable memory increment mode
    DMA2_Stream5->CR |= (1 << 8); //Enable circular mode
    //Configure FIFO usage
    //Configure transfer direction, peripheral and memory fixed mode, single transactions, data widths
    //Enable stream in DMA_SxCR register
    DMA2_Stream5->CR |= 1;
    //Activate stream
    // Set transfer direciton to memory-to-peripheral (01) in DMA_SxCR
    // Set dma 2 channel 6 to use stream 5
    
    TIM1->PSC = 0;
    TIM1->ARR = 25;

    TIM1->DCR |= 13; //Set offset to TIMx_CCR1
    TIM1->DIER |= (1 << 8); //Enable DMA request on update
    TIM1->CCMR1 |= (0b110 << 4); // Set PWM mode 1
    TIM1->CCMR1 |= (1 << 3); // Enable preload register
    TIM1->CCER |= TIM_CCER_CC1E;  // Enable output
    TIM1->CR1 |= (1 << 7);        // Enable auto-reload preload register
    TIM1->CCR1 = 10;
    TIM1->BDTR |= TIM_BDTR_MOE; // Enable main output

    TIM1->CR1 |= 1; // Start the counter

}