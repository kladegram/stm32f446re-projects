#ifndef LED_H
#define LED_H

#include <stdbool.h>

/*
 * Green user LED (LD2) on the NUCLEO-F446RE: PA5, active high.
 * Copy led.c and led.h together and add led.c to the project's build.
 * Requires ST's STM32F4 CMSIS headers and the STM32F446xx device define.
 */

/* Enable the GPIO clock and configure PA5 as an output, initially off. */
void LED_Init(void);

/* Call LED_Init() before toggling or reading the LED state. */
void LED_Toggle(void);

/* Return true when PA5's output bit is set (LED on). */
bool LED_IsOn(void);

#endif
