#include "gpio.h"

void gpio_set_mode(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t mode) {
	set_bits(&gpiox->MODER, mode, 2 * pin, 2 * (pin + 1));
}

void gpio_set_otype(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t otype) {
	gpiox->OTYPER |= (otype << pin);
}

void gpio_set_ospeed(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t ospeed) {
	set_bits(&gpiox->OSPEEDR, ospeed, 2 * pin, 2 * (pin + 1));
}

void gpio_set_af(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t af) {
	uint8_t af_index = 0;

	if (pin >= 8) {
		pin -= 8;
		af_index = 1;
	}

	set_bits(&gpiox->AFR[af_index], af, 4 * pin, 4 * (pin + 1));
}

void gpio_set_pupd(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t pupd) {
	set_bits(gpiox->PUPDR, pupd, 2 * pin, 2 * (pin + 1));
}

void gpio_write_pin(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t state) {
	if (state) {
		gpiox->BSRR = (1U << pin);
	} else {
		gpiox->BSRR = (1U << (pin + 16));
	}
}

void gpio_read_pin(GPIO_TypeDef *gpiox, uint8_t pin, uint8_t *state) {
	*state = (gpiox->ODR >> pin) & 0x01;
}
