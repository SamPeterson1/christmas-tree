#ifndef RCC_H
#define RCC_H

#include "stm32f4xx.h"

#define SYS_CLK 16000000U
#define APB1_CLK SYS_CLK
#define SYSTICK_CLK (SYS_CLK / 8)

#define RCC_AHB1_EN (&RCC->AHB1ENR)
#define RCC_APB1_EN (&RCC->APB1ENR)
#define RCC_APB2_EN (&RCC->APB2ENR)

#define RCC_AHB1_GPIOA (1U << 0)
#define RCC_AHB1_GPIOB (1U << 1)
#define RCC_AHB1_GPIOC (1U << 2)
#define RCC_AHB1_GPIOD (1U << 3)
#define RCC_AHB1_GPIOE (1U << 4)
#define RCC_AHB1_GPIOF (1U << 5)
#define RCC_AHB1_GPIOG (1U << 6)
#define RCC_AHB1_GPIOH (1U << 7)
#define RCC_AHB1_GPIOI (1U << 8)
#define RCC_AHB1_GPIOJ (1U << 9)
#define RCC_AHB1_GPIOK (1U << 10)
#define RCC_AHB1_DMA1EN (1U << 21)
#define RCC_AHB1_DMA2EN (1U << 22)

#define RCC_APB1_USART2 (1U << 17)
#define RCC_APB1_USART3 (1U << 18)
#define RCC_APB1_UART4 (1U << 19)
#define RCC_APB1_UART5 (1U << 20)
#define RCC_APB1_I2C1 (1U << 21)
#define RCC_APB1_I2C2 (1U << 22)
#define RCC_APB1_I2C3 (1U << 23)

#define RCC_APB2_TIM1 (1U << 0)
#define RCC_APB2_USART1 (1U << 4)
#define RCC_APB2_ADC1 (1U << 8)
#define RCC_APB2_ADC2 (1U << 9)
#define RCC_APB2_ADC3 (1U << 10)
#define RCC_APB2_SYSCFG (1U << 14)

void rcc_enable(volatile uint32_t *rcc_enr, uint32_t periph);
void rcc_init(void);

#endif
