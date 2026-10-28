#include "ring_buffer.h"

bool ring_buff_push(ring_buff_t *r, uint8_t value) {
  uint32_t next_head = (r->head + 1) % RING_BUFF_SIZE;

  if (next_head == r->tail) {
    return false;
  }

  r->buffer[r->head] = value;
  r->head = next_head;

  return true;
}

bool ring_buff_pop(ring_buff_t *r, uint8_t *value) {
  if (ring_buff_is_empty(r)) {
    return false;
  }

  *value = r->buffer[r->tail];
  r->tail = (r->tail + 1) % RING_BUFF_SIZE;

  return true;
}

bool ring_buff_is_empty(ring_buff_t *r) {
  return r->tail == r->head;
}
