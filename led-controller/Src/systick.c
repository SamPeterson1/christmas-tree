#include "systick.h"

void systick_delay_ms(uint32_t ms) {
	while (ms) {
		uint32_t delay = ms;

		if (ms > 1000) {
			delay = 1000;
		}

		uint32_t ticks = delay * (SYS_CLK / 1000U);

		SysTick->LOAD = ticks & SYSTICK_VALUE_MSK;
		SysTick->VAL &= ~SYSTICK_VALUE_MSK;
		SysTick->CTRL |= SYSTICK_ENABLE | SYSTICK_INTERNAL_CLK;

		ms -= delay;

		while ((SysTick->CTRL & SYSTICK_COUNTFLAG) == 0);
	}

}
