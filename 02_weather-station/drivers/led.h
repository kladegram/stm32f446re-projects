#pragma once

#include <stdbool.h>
#include <stdint.h>

void led_init(void);
void led_toggle(void);
void led_turn_on(void);
void led_turn_off(void);
bool led_is_on(void);
