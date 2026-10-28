#pragma once

#include "./i2c.h"

void aht20_init(uint8_t address);
void aht20_begin(uint32_t now_ms);
void aht20_update(uint32_t now_ms);
bool aht20_take_reading(float[]);
bool aht20_is_busy(void);
