#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*
 * TIM2 periodic interrupts on the STM32F446, using the default 16 MHz timer clock.
 * Copy timer.c and timer.h together and add timer.c to the project's build.
 * Requires ST's STM32F4 CMSIS headers and the STM32F446xx device define.
 * startup.s must connect external IRQ 28 to TIM2_IRQHandler.
 */

/* Called inside the timer interrupt after acknowledging the update flag. */
typedef void (*timer_callback_t)(void);

/* Call once with a valid callback to configure and start TIM2; period_ms must be >= 1. */
void TIM2_Init(uint32_t period_ms, timer_callback_t on_update);

/* Call TIM2_Init() first. Change the period and restart its interval only if needed. */
void TIM2_SetPeriod(uint32_t period_ms);

#endif
