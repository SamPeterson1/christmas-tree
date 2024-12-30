#ifndef GPIO_H
#define GPIO_H

#include "stm32f4xx.h"
#include "util.h"

#define GPIO_HIGH 	(1)
#define GPIO_LOW 	(0)

#define GPIO_PORT_A (GPIOA)
#define GPIO_PORT_B (GPIOB)
#define GPIO_PORT_C (GPIOC)
#define GPIO_PORT_D (GPIOD)
#define GPIO_PORT_E (GPIOE)
#define GPIO_PORT_F (GPIOF)
#define GPIO_PORT_G (GPIOG)
#define GPIO_PORT_H (GPIOH)
#define GPIO_PORT_I (GPIOI)
#define GPIO_PORT_J (GPIOJ)
#define GPIO_PORT_K (GPIOK)

#define GPIO_PIN_0 	(0)
#define GPIO_PIN_1 	(1)
#define GPIO_PIN_2 	(2)
#define GPIO_PIN_3 	(3)
#define GPIO_PIN_4 	(4)
#define GPIO_PIN_5 	(5)
#define GPIO_PIN_6 	(6)
#define GPIO_PIN_7 	(7)
#define GPIO_PIN_8 	(8)
#define GPIO_PIN_9 	(9)
#define GPIO_PIN_10 (10)
#define GPIO_PIN_11 (11)
#define GPIO_PIN_12 (12)
#define GPIO_PIN_13 (13)
#define GPIO_PIN_14 (14)
#define GPIO_PIN_15 (15)

#define GPIO_MODE_INPUT 	(0b00)
#define GPIO_MODE_OUTPUT 	(0b01)
#define GPIO_MODE_AF 		(0b10)
#define GPIO_MODE_ANALOG 	(0b11)

#define GPIO_OSPEED_LOW 	(0b00)
#define GPIO_OSPEED_MED 	(0b01)
#define GPIO_OSPEED_HIGH 	(0b10)
#define GPIO_OSPEED_VHIGH 	(0b11)

#define GPIO_AF0 	(0b0000)
#define GPIO_AF1 	(0b0001)
#define GPIO_AF2 	(0b0010)
#define GPIO_AF3 	(0b0011)
#define GPIO_AF4 	(0b0100)
#define GPIO_AF5 	(0b0101)
#define GPIO_AF6 	(0b0110)
#define GPIO_AF7 	(0b0111)
#define GPIO_AF8 	(0b1000)
#define GPIO_AF9 	(0b1001)
#define GPIO_AF10 	(0b1010)
#define GPIO_AF11 	(0b1011)
#define GPIO_AF12 	(0b1100)
#define GPIO_AF13 	(0b1101)
#define GPIO_AF14 	(0b1110)
#define GPIO_AF15 	(0b1111)

#define GPIO_PU (0b01)
#define GPIO_PD (0b10)

#define GPIO_PUSH_PULL 0
#define GPIO_OPEN_DRAIN 1

void gpio_set_mode(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t mode);
void gpio_set_otype(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t otype);
void gpio_set_ospeed(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t ospeed);
void gpio_set_af(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t af);
void gpio_set_pupd(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t pupd);
void gpio_write_pin(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t state);
void gpio_read_pin(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t *state);

#endif
