#include "effect.h"

uint8_t init_off_params(uint8_t *params);
void update_off(LEDController *led_controller);

void init_off_effect(Effect *effect) {
    const static char *effect_name = "Off";

    effect->id = OFF_EFFECT_ID;
    effect->version = OFF_EFFECT_VERSION;
    
    effect->name_len = strlen(effect_name);
    strcpy(effect->name, effect_name);
    effect->n_params = 0;
    effect->init_params = init_off_params;
    effect->update = update_off;
}

uint8_t init_off_params(uint8_t *params) {
    return ERR_OK;
}

void update_off(LEDController *led_controller) {
    for (int i = 0; i < N_LEDS; i ++) {
        led_set(led_controller, i, (RGBColor) {0, 0, 0});
    }
}

