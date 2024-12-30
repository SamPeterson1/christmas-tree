#ifndef COLOR_H
#define COLOR_H

#include "stdint.h"

typedef struct {
	uint8_t h;
	uint8_t s;
	uint8_t v;
} HSVColor;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGBColor;

RGBColor hsv_to_rgb(HSVColor hsv);

#endif