#include "ring_buffer.h"

void ring_buffer_init(RingBuffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;

    memset(buffer->data, 0, RING_BUFFER_SIZE);
}

uint16_t ring_buffer_size(RingBuffer *buffer) {
    if (buffer->head >= buffer->tail) {
        return buffer->head - buffer->tail;
    } else {
        return RING_BUFFER_SIZE - (buffer->tail - buffer->head);
    }
}

void ring_buffer_read(RingBuffer *buffer, uint8_t *data, uint16_t size) {
    for (int i = 0; i < size; i++) {
        data[i] = buffer->data[buffer->tail];
        buffer->tail = (buffer->tail + 1) % RING_BUFFER_SIZE;
    }
}

void ring_buffer_write(RingBuffer *buffer, uint8_t *data, uint16_t size) {
    for (int i = 0; i < size; i++) {
        buffer->data[buffer->head] = data[i];
        buffer->head = (buffer->head + 1) % RING_BUFFER_SIZE;
    }
}