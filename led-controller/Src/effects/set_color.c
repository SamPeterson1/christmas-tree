#include "effect.h"

uint8_t init_set_color_params(uint8_t *params);
void update_set_color(LEDController *led_controller);

struct {
    RGBColor color;
} set_color_params;

void init_set_color_effect(Effect *effect) {
    const static char *effect_name = "Set Color";
    const static char *param_1_name = "Color";

    effect->id = SET_COLOR_EFFECT_ID;
    effect->version = SET_COLOR_EFFECT_VERSION;

    effect->name_len = strlen(effect_name);
    strcpy(effect->name, effect_name);
    effect->n_params = 1;

    effect->params[0].type = PARAM_TYPE_COLOR;
    effect->params[0].name_len = strlen(param_1_name);
    strcpy(effect->params[0].name, param_1_name);

    effect->init_params = init_set_color_params;
    effect->update = update_set_color;
}

uint8_t init_set_color_params(uint8_t *params) {
    uint8_t i = 0;

    set_color_params.color = next_color(params, &i);
    
    return ERR_OK;
}

void update_set_color(LEDController *led_controller) {
    for (int i = 0; i < N_LEDS; i ++) {
        led_set(led_controller, i, set_color_params.color);
    }
}

