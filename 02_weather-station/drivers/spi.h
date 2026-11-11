#pragma once

#include "./timer.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t sck_pin;
  uint8_t mosi_pin;
  uint8_t baud_div;
} spi2_config_t;

void spi2_init(const spi2_config_t *config);
bool spi2_write(unsigned char);