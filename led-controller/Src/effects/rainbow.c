#include "effect.h"

uint8_t init_rainbow_params(uint8_t *params);
void update_rainbow(LEDController *led_controller);

struct {
    float speed;
    uint8_t saturation;
    uint8_t brightness;
    float wavelength;
    uint8_t axis;
} rainbow_params;


void init_rainbow_effect(Effect *effect) {
    const static char *effect_name = "Rainbow";

    const static char *param_1_name = "Speed";
    const static char *param_2_name = "Saturation";
    const static char *param_3_name = "Brightness";
    const static char *param_4_name = "Wavelength";
    const static char *param_5_name = "Axis";

    effect->id = RAINBOW_EFFECT_ID;
    effect->version = RAINBOW_EFFECT_VERSION;

    effect->name_len = strlen(effect_name);
    strcpy(effect->name, effect_name);
    effect->n_params = 5;

    effect->params[0].type = PARAM_TYPE_FLOAT;
    effect->params[0].name_len = strlen(param_1_name);
    strcpy(effect->params[0].name, param_1_name);

    effect->params[1].type = PARAM_TYPE_UINT8;
    effect->params[1].name_len = strlen(param_2_name);
    strcpy(effect->params[1].name, param_2_name);

    effect->params[2].type = PARAM_TYPE_UINT8;
    effect->params[2].name_len = strlen(param_3_name);
    strcpy(effect->params[2].name, param_3_name);

    effect->params[3].type = PARAM_TYPE_FLOAT;
    effect->params[3].name_len = strlen(param_4_name);
    strcpy(effect->params[3].name, param_4_name);

    effect->params[4].type = PARAM_TYPE_UINT8;
    effect->params[4].name_len = strlen(param_5_name);
    strcpy(effect->params[4].name, param_5_name);

    effect->init_params = init_rainbow_params;
    effect->update = update_rainbow;
}

uint8_t init_rainbow_params(uint8_t *params) {
    uint8_t i = 0;

    rainbow_params.speed = next_float(params, &i);
    rainbow_params.saturation = next_u8(params, &i);
    rainbow_params.brightness = next_u8(params, &i);
    rainbow_params.wavelength = next_float(params, &i);
    rainbow_params.axis = next_u8(params, &i);

    if (rainbow_params.axis > 2) {
        return ERR_PARAM_OUT_OF_BOUNDS;
    }
    
    return ERR_OK;
}

void update_rainbow(LEDController *led_controller) {
    static float t = 0;

    for (int i = 0; i < N_LEDS; i ++) {
        float x = led_positions[i][rainbow_params.axis];

        uint8_t h = ((uint32_t) (x * rainbow_params.wavelength + 255 + t)) % 255;
        RGBColor color = hsv_to_rgb((HSVColor) {h, rainbow_params.saturation, rainbow_params.brightness});

        led_set(led_controller, i, color);
    }

    t += rainbow_params.speed;
}

