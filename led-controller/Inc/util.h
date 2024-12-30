#ifndef UTIL_H
#define UTIL_H

#include "stdint.h"

void set_bits(volatile uint32_t *dest, uint32_t value, uint8_t start, uint8_t end);

#endif
