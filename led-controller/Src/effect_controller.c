#include "effect_controller.h"

void list_effects(EffectController *controller);    
uint16_t get_list_effects_size(EffectController *controller);
uint8_t get_n_effects(EffectController *controller);
void list_effect(EffectController *controller, uint8_t effect_id);
void send_params(Effect *effect);
int32_t get_param_bytes(uint8_t param_type);
int32_t calculate_param_bytes(Effect *effect);
void listen_next_effect(EffectController *controller, RingBuffer *uart_rx_buffer);
void listen_effect_params(EffectController *controller, RingBuffer *uart_rx_buffer);

/* 
 * Initiaize all effects and effect controller
 * Any new effects should be initialized here
 */
void init_effects(EffectController *controller) {
    controller->current_effect_id = 0x00;
    controller->expected_param_bytes = -1;

    led_init(&controller->led_controller);

    for (int i = 0; i < MAX_N_EFFECTS; i++) {
        controller->effects[i].id = 0xFF;
    }

    init_off_effect(&controller->effects[OFF_EFFECT_ID]);
    init_set_color_effect(&controller->effects[SET_COLOR_EFFECT_ID]);
    init_rainbow_effect(&controller->effects[RAINBOW_EFFECT_ID]);
}

/* 
 * Listen over uart for new effects
 * Call the update function of the current effect
 * Push updates to the DMA buffer 
 */
void update_effects(EffectController *controller, RingBuffer *uart_rx_buffer) {
    if (controller->expected_param_bytes == -1) {
        listen_next_effect(controller, uart_rx_buffer);
    } else {
        listen_effect_params(controller, uart_rx_buffer);
    }

    controller->effects[controller->current_effect_id].update(&controller->led_controller);

    led_update(&controller->led_controller);
}

/* Send all effects and their parameters over UART */
void list_effects(EffectController *controller) {
    uint16_t msg_size = get_list_effects_size(controller);
    uart_tx(USART3, (uint8_t *) &msg_size, 2);

    uint8_t n_effects = get_n_effects(controller);
    uart_tx(USART3, &n_effects, 1);

    for (int i = 0; i < MAX_N_EFFECTS; i ++) {
        if (controller->effects[i].id == 0xFF) continue;

        uart_tx(USART3, &controller->effects[i].id, 1);

        uart_tx(USART3, &controller->effects[i].version, 1);

        uint8_t effect_name_len = controller->effects[i].name_len;
        uart_tx(USART3, &effect_name_len, 1);

        uart_tx(USART3, (uint8_t *) controller->effects[i].name, effect_name_len);

        uint8_t n_params = controller->effects[i].n_params;
        uart_tx(USART3, &n_params, 1);

        for (int j = 0; j < n_params; j++) {
            EffectParam *param = &controller->effects[i].params[j];

            uint8_t param_name_len = param->name_len;
            uart_tx(USART3, &param_name_len, 1);

            uart_tx(USART3, (uint8_t *) param->name, param_name_len);

            uart_tx(USART3, &param->type, 1);
        }
    }
}

/* Calculate the size of the list effects message */
uint16_t get_list_effects_size(EffectController *controller) {
    uint16_t msg_size = 1;

    for (int i = 0; i < MAX_N_EFFECTS; i ++) {
        Effect *effect = &controller->effects[i];

        if (effect->id == 0xFF) continue;

        msg_size += 1 + 1 + 1 + effect->name_len + 1;

        for (int j = 0; j < effect->n_params; j++) {
            msg_size += 1 + effect->params[j].name_len + 1;
        }
    }

    return msg_size;
}

/* Get the number of effects */
uint8_t get_n_effects(EffectController *controller) {
    uint8_t n_effects = 0;

    for (int i = 0; i < MAX_N_EFFECTS; i ++) {
        if (controller->effects[i].id == i) {
            n_effects++;
        }
    }

    return n_effects;
}

/* Send a singular effect and its parameters over UART */
void list_effect(EffectController *controller, uint8_t effect_id) {
    Effect *effect = &controller->effects[effect_id];

    if (effect->id == 0xFF) return;

    uart_tx(USART3, effect->id, 1);

    uart_tx(USART3, effect->version, 1);

    uint8_t effect_name_len = effect->name_len;
    uart_tx(USART3, &effect_name_len, 1);

    uart_tx(USART3, (uint8_t *) effect->name, effect_name_len);

    send_params(effect);    
}

/* Send the parameters of an effect over UART */
void send_params(Effect *effect) {
    int8_t n_params = effect->n_params;
    uart_tx(USART3, &n_params, 1);

    for (int j = 0; j < n_params; j++) {
        EffectParam *param = &effect->params[j];

        uint8_t param_name_len = param->name_len;
        uart_tx(USART3, &param_name_len, 1);

        uart_tx(USART3, (uint8_t *) param->name, param_name_len);

        uart_tx(USART3, &param->type, 1);
    }
}

/* Return the number of bytes to store a parameter type */
int32_t get_param_bytes(uint8_t param_type) {
    switch (param_type) {
        case PARAM_TYPE_UINT8:
            return 1;
        case PARAM_TYPE_UINT32:
            return 4;
        case PARAM_TYPE_INT8:
            return 1;
        case PARAM_TYPE_INT32:
            return 4;
        case PARAM_TYPE_FLOAT:
            return 4;
        case PARAM_TYPE_COLOR:
            return 3;
        case PARAM_TYPE_BOOLEAN:
            return 1;
    }

    return 0;
}

/* Calculate the number of bytes to store all parameters of an effect */
int32_t calculate_param_bytes(Effect *effect) {
    int32_t bytes = 0;

    for (int i = 0; i < effect->n_params; i ++) {
        bytes += get_param_bytes(effect->params[i].type);
    }

    return bytes;
}

/* 
 * Listen for a byte to be sent and check if it is a valid effect id
 * If it is, set the pending effect id and calculate the number of bytes to expect
 * If it is not, send an error message
 */
void listen_next_effect(EffectController *controller, RingBuffer *uart_rx_buffer) {
    if (ring_buffer_size(uart_rx_buffer) == 0) {
        return;
    }

    uint8_t effect_id;
    ring_buffer_read(uart_rx_buffer, &effect_id, 1);

    if (effect_id == 0xFF) {
        list_effects(controller);
    } else if (controller->effects[effect_id].id == effect_id) {
        controller->pending_effect_id = effect_id;
        controller->expected_param_bytes = calculate_param_bytes(&controller->effects[effect_id]);
    } else {
        uint16_t msg_size = 1;
        uint8_t error_code = ERR_INVALID_EFFECT_ID;

        uart_tx(USART3, (uint8_t *) &msg_size, 2);
        uart_tx(USART3, &error_code, 1);
    }
}

/*
 * Listen for the parameters of the pending effect
 * If the correct number of bytes have been sent, call the init_params function of the effect
 * Send an error message if the effect version is incorrect
 * Send an error message if the effect init_params function returns an error
 * If no error occurs, set the current effect id to the pending effect id and reset the expected param bytes
 */
void listen_effect_params(EffectController *controller, RingBuffer *uart_rx_buffer) {
    if (ring_buffer_size(uart_rx_buffer) < controller->expected_param_bytes + 1) {
        return;
    }

    uint8_t effect_version = controller->effects[controller->pending_effect_id].version;

    uint8_t sent_version = 0;
    ring_buffer_read(uart_rx_buffer, &sent_version, 1);

    uint8_t effect_id = controller->pending_effect_id;
    ring_buffer_read(uart_rx_buffer, controller->effect_param_buffer, controller->expected_param_bytes);

    uint8_t error_code = ERR_OK;

    if (sent_version != effect_version) {
        error_code = ERR_INVALID_EFEFCT_VERSION;
    } else {
        error_code = controller->effects[effect_id].init_params(controller->effect_param_buffer);
    }

    uint16_t msg_size = 1;

    uart_tx(USART3, (uint8_t *) &msg_size, 2);
    uart_tx(USART3, &error_code, 1);

    if (error_code == ERR_OK) {
        controller->current_effect_id = effect_id;
    }

    controller->expected_param_bytes = -1;
}