#include "util.h"

void set_bits(volatile uint32_t *dest, uint32_t value, uint8_t start, uint8_t end) {
	int i = 0;

	for (int bit = start; bit < end; bit ++) {
		if (value & (1U << i++)) {
			*dest |= (1U << bit);
		} else {
			*dest &= ~(1U << bit);
		}
	}
}
