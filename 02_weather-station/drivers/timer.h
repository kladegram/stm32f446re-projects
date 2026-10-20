#pragma once

#include <stddef.h>
#include <stdint.h>

void timer_init(void);
void timer_start(void);
void timer_clear_update_flag(void);
void timer_enable_interrupt(void);
uint32_t timer_get_ms(void);
