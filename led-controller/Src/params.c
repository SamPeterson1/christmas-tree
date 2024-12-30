#include "params.h"

uint8_t next_u8(uint8_t *params, uint8_t *index) {
    uint8_t value;
    memcpy(&value, params + *index, 1);
    (*index)++;

    return value;
}

uint32_t next_u32(uint8_t *params, uint8_t *index) {
    uint32_t value;
    memcpy(&value, params + *index, 4);
    *index += 4;

    return value;
}

int8_t next_i8(uint8_t *params, uint8_t *index) {
    int8_t value;
    memcpy(&value, params + *index, 1);
    (*index)++;

    return value;
}

int32_t next_i32(uint8_t *params, uint8_t *index) {
    int32_t value;
    memcpy(&value, params + *index, 4);
    *index += 4;

    return value;
}

float next_float(uint8_t *params, uint8_t *index) {
    float value;
    memcpy(&value, params + *index, 4);
    *index += 4;

    return value;
}

RGBColor next_color(uint8_t *params, uint8_t *index) {
    RGBColor color;
    
    color.r = next_u8(params, index);
    color.g = next_u8(params, index);
    color.b = next_u8(params, index);

    return color;
}

uint8_t next_bool(uint8_t *params, uint8_t *index) {
    return next_u8(params, index);
}