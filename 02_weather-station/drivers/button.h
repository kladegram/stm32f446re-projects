#pragma once

#include <stdbool.h>
#include <stdint.h>

void button_init(void);
bool button_is_pressed(void);
bool button_has_interrupt_pending(void);
void button_clear_interrupt_flag(void);
