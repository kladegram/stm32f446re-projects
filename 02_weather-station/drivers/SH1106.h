#pragma once

#include "../lib/Font_8x8.h"
#include "./spi.h"

typedef struct {
  uint8_t res_pin;
  uint8_t dc_pin;
  uint8_t cs_pin;
} sh1106_config_t;

void sh1106_init(const sh1106_config_t *config);
bool sh1106_write_command(unsigned char);
bool sh1106_write_data(unsigned char);
bool h1106_write_string(const unsigned char *s);
bool h1106_set_pos(uint32_t page, uint32_t column);
bool sh1106_clear(void);
bool sh1106_enter_sleep(void);
bool sh1106_exit_sleep(void);
