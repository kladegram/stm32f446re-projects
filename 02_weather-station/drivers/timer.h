#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void timer_init(void);
void timer_clear_update_flag(void);
void timer_enable_interrupt(void);
uint32_t timer_get_ms(void);
void timer_delay_ms(uint32_t duration_ms);
bool timer_wait_for(volatile const uint32_t *reg, uint32_t mask, bool wanted_set,
                    uint32_t timeout_ms);