#ifndef PWM_H
#define PWM_H

#include "stm32f4xx.h"
#include "gpio.h"

void pwm_init(uint16_t *dma_buf, uint16_t size);
#endif