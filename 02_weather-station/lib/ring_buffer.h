/**
 * Very basic FIFO
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

// Note: Usable count is `RING_BUFF_SIZE - 1`
#define RING_BUFF_SIZE 32U

typedef struct {
  uint8_t buffer[RING_BUFF_SIZE];
  uint32_t head;
  uint32_t tail;
} ring_buff_t;

bool ring_buff_push(ring_buff_t *r, uint8_t value);
bool ring_buff_pop(ring_buff_t *r, uint8_t *value);
