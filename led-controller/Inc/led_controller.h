#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "stdint.h"
#include "color.h"
#include "pwm.h"

#define N_LEDS 450
#define N_BLANK_FRAMES 100


extern const float led_positions[450][3];

typedef struct {
    RGBColor colors[N_LEDS];
    uint16_t pwm_dma_buf[N_LEDS * 24 + N_BLANK_FRAMES * 24];
} LEDController;

void led_init(LEDController *controller);
void led_set(LEDController *controller, int i, RGBColor color);
void led_update(LEDController *controller);

#endif