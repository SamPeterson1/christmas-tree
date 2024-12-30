#include "uart.h"

uint16_t compute_uart_div(uint32_t periph_clk, uint32_t baud_rate);

void usart3_init() {
	gpio_set_mode(USART3_TX_PORT, USART3_TX_PIN, GPIO_MODE_AF);
	gpio_set_af(USART3_TX_PORT, USART3_TX_PIN, GPIO_AF7);

	gpio_set_mode(USART3_RX_PORT, USART3_RX_PIN, GPIO_MODE_AF);
	gpio_set_af(USART3_RX_PORT, USART3_RX_PIN, GPIO_AF7);

	USART3->CR1 |= USART_EN | USART_TX_EN | USART_RX_EN | USART_PCE | USART_RXNE_IE | USART_M;
	USART3->CR1 &= ~USART_PS;

	USART3->BRR = compute_uart_div(APB1_CLK, USART3_BAUD_RATE);
}

void uart_rx(USART_TypeDef *usartx, uint8_t *data, uint16_t size) {
	for (int i = 0; i < size; i ++) {
		while(!(usartx->SR & USART_RXNE));
		data[i] = usartx->DR;
	}
}

void uart_tx(USART_TypeDef *usartx, uint8_t *data, uint16_t size) {
	for (int i = 0; i < size; i ++) {
		while(!(usartx->SR & USART_TXE));
		usartx->DR = data[i];
	}
}

uint16_t compute_uart_div(uint32_t periph_clk, uint32_t baud_rate) {
	return (periph_clk + (baud_rate / 2U)) / baud_rate;
}

