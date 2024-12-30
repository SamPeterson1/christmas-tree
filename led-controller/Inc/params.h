#ifndef PARAMS_H
#define PARAMS_H

#include "stdint.h"
#include "color.h"

uint8_t next_u8(uint8_t *params, uint8_t *index);
uint32_t next_u32(uint8_t *params, uint8_t *index);
int8_t next_i8(uint8_t *params, uint8_t *index);
int32_t next_i32(uint8_t *params, uint8_t *index);
float next_float(uint8_t *params, uint8_t *index);
RGBColor next_color(uint8_t *params, uint8_t *index);
uint8_t next_bool(uint8_t *params, uint8_t *index);

#endif