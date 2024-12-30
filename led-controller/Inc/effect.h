#ifndef EFFECT_H
#define EFFECT_H

#include "led_controller.h"
#include "params.h"

#define EFFECT_NAME_MAX_LEN 32
#define PARAM_NAME_MAX_LEN 32
#define MAX_N_PARAMS 8

#define OFF_EFFECT_ID 0x00
#define OFF_EFFECT_VERSION 0x00

#define SET_COLOR_EFFECT_ID 0x01
#define SET_COLOR_EFFECT_VERSION 0x00

#define RAINBOW_EFFECT_ID 0x02
#define RAINBOW_EFFECT_VERSION 0x00

typedef struct {
	uint8_t type;
    uint8_t name_len;
	char name[PARAM_NAME_MAX_LEN];
} EffectParam;

typedef struct {
	uint8_t id;
    uint8_t name_len;
	uint8_t version;
    char name[EFFECT_NAME_MAX_LEN];
	uint8_t n_params;
	EffectParam params[MAX_N_PARAMS];
	uint8_t(*init_params)(uint8_t *);
	void(*update)(LEDController *);
} Effect;

#include "effect_controller.h"

void init_off_effect(Effect *effect);
void init_set_color_effect(Effect *effect);
void init_rainbow_effect(Effect *effect);

#endif