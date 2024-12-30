#include "rcc.h"

void rcc_enable(volatile uint32_t *rcc_enr, uint32_t periph) {
	*rcc_enr |= periph;
}

void rcc_init(void) {
	rcc_enable(RCC_APB1_EN, RCC_APB1_USART3);
	rcc_enable(RCC_APB2_EN, RCC_APB2_TIM1 | RCC_APB2_ADC3 | RCC_APB2_SYSCFG | RCC_APB2_USART1);
	rcc_enable(RCC_AHB1_EN, RCC_AHB1_DMA2EN | RCC_AHB1_GPIOA | RCC_AHB1_GPIOB | RCC_AHB1_GPIOC | RCC_AHB1_GPIOD | RCC_AHB1_GPIOE | RCC_AHB1_GPIOF);
}
