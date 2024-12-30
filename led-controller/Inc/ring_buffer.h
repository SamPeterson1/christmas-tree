#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "stdint.h"
#include "string.h"

#define RING_BUFFER_SIZE 1024

typedef struct {
    uint16_t head;
    uint16_t tail;
    uint8_t data[RING_BUFFER_SIZE];
} RingBuffer;

void ring_buffer_init(RingBuffer *buffer);
uint16_t ring_buffer_size(RingBuffer *buffer);
void ring_buffer_read(RingBuffer *buffer, uint8_t *data, uint16_t size);
void ring_buffer_write(RingBuffer *buffer, uint8_t *data, uint16_t size);

#endif