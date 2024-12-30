#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm32f4xx.h"
#include "rcc.h"

#define SYSTICK_ENABLE (1U << 0)
#define SYSTICK_INTERNAL_CLK (1U << 2)
#define SYSTICK_VALUE_MSK 0xFFFFFF
#define SYSTICK_COUNTFLAG (1U << 16)

void systick_delay_ms(uint32_t ms);

#endif
