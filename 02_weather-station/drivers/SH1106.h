#pragma once

#include "../lib/Font_8x8.h"
#include "./spi.h"

void sh1106_init(void);
bool sh1106_write_command(unsigned char);
bool sh1106_write_data(unsigned char);
bool h1106_write_string(const unsigned char *s);
bool h1106_set_pos(uint32_t page, uint32_t column);
bool sh1106_clear(void);