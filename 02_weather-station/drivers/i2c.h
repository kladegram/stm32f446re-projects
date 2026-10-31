#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void i2c_init(void);
void i2c_reset(void);

bool i2c_generate_start(uint8_t);
void i2c_generate_stop(void);

bool i2c_write(uint8_t, const uint8_t *, uint16_t);
bool i2c_read(uint8_t, uint8_t *, uint16_t);
