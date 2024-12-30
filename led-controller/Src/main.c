#include "stm32f4xx.h"
#include "uart.h"
#include "rcc.h"
#include "systick.h"
#include "ring_buffer.h"
#include "effect_controller.h"

RingBuffer uart3_rx_buffer;

void SystemInit(void) {
	// Enable FPU
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
}

void USART3_IRQHandler(void) {
	if (USART3->SR & USART_RXNE) {
		uint8_t data = USART3->DR;
		ring_buffer_write(&uart3_rx_buffer, &data, 1);
	}
}

int main(void) {
	rcc_init();

	//Enable USART3 interrupt
	NVIC->ISER[1] |= (1U << 7);

	usart3_init();

	ring_buffer_init(&uart3_rx_buffer);

	EffectController effect_controller;
	init_effects(&effect_controller);

	while(1) {
		update_effects(&effect_controller, &uart3_rx_buffer);
		systick_delay_ms(10);
	}
}
