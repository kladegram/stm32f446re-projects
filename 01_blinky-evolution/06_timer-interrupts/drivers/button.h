#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

/*
 * Blue user button (B1) on the NUCLEO-F446RE: PC13, active low.
 * Copy button.c and button.h together and add button.c to the project's build.
 * Requires ST's STM32F4 CMSIS headers and the STM32F446xx device define.
 * startup.s must connect external IRQ 40 to EXTI15_10_IRQHandler.
 */

/*
 * Receives the initial state during BUTTON_Init(), then either edge from the interrupt.
 * true = pressed, false = released. Initialize callback dependencies first.
 * Keep the callback short. Contact bounce can produce multiple notifications.
 */
typedef void (*button_callback_t)(bool pressed);

/*
 * Configure GPIOC and EXTI13, report the initial state, then enable button interrupts.
 * Call once with a valid callback.
 */
void BUTTON_Init(button_callback_t on_change);

/* Call BUTTON_Init() first; return true while PC13 reads low. */
bool BUTTON_IsPressed(void);

#endif
