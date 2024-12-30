#ifndef EFFECT_CONTROLLER_H
#define EFFECT_CONTROLLER_H

#include "stdint.h"
#include "ring_buffer.h"
#include "uart.h"
#include "led_controller.h"
#include "effect.h"

#define MAX_N_EFFECTS 256

#define ERR_OK 0x00
#define ERR_INVALID_EFFECT_ID 0x01
#define ERR_INVALID_EFEFCT_VERSION 0x02
#define ERR_PARAM_OUT_OF_BOUNDS 0x03

#define PARAM_TYPE_UINT8 0x00
#define PARAM_TYPE_UINT32 0x01
#define PARAM_TYPE_INT8 0x02
#define PARAM_TYPE_INT32 0x03
#define PARAM_TYPE_FLOAT 0x04
#define PARAM_TYPE_COLOR 0x05
#define PARAM_TYPE_BOOLEAN 0x06

typedef struct {
    LEDController led_controller;

    uint8_t current_effect_id;
    Effect effects[MAX_N_EFFECTS];

    uint8_t effect_param_buffer[MAX_N_PARAMS * 4];
    uint8_t pending_effect_id;
    int32_t expected_param_bytes;
} EffectController;

void init_effects(EffectController *controller);
void update_effects(EffectController *controller, RingBuffer *uart_rx_buffer);

#endif