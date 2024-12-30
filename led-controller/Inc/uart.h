#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"
#include "rcc.h"
#include "pins.h"
#include "gpio.h"

#define USART_EN	(1U << 13)
#define USART_M		(1U << 12)
#define USART_PCE   (1U << 10)
#define USART_PS    (1U << 9)
#define USART_TX_EN	(1U << 3)
#define USART_RX_EN	(1U << 2)
#define USART_RXNE_IE (1U << 5)

#define USART_TXE (1U << 7)
#define USART_RXNE (1U << 5)

#define USART1_BAUD_RATE 115200
#define USART3_BAUD_RATE 115200

void usart1_init();
void usart3_init();
void uart_rx(USART_TypeDef *usartx, uint8_t *data, uint16_t size);
void uart_tx(USART_TypeDef *usartx, uint8_t *data, uint16_t size);

#endif
