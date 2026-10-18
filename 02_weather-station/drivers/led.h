#pragma once

#include <stdbool.h>
#include <stdint.h>

void led_init(void);
void led_toggle(void);
bool led_is_on(void);
