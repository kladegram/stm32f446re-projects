/*
 * Button-controlled LED blink using TIM2 interrupts on the NUCLEO-F446RE.
 * Toggle every 1000 ms normally, or every 100 ms while the blue B1 button is held.
 *
 * Reference: ST RM0390, RCC, GPIO, SYSCFG, EXTI and timer registers;
 * NUCLEO-F446RE board schematic, user button B1 on PC13.
 *
 * Reusable modules in drivers/:
 *   led.c / led.h: user LED setup, toggle and state reading.
 *   uart.c / uart.h: USART2 transmit and string output.
 *   button.c / button.h: button setup and press/release callbacks.
 *   timer.c / timer.h: TIM2 setup, period changes and periodic callbacks.
 *
 * Steps:
 *   1. Initialize the LED and UART.
 *   2. Start TIM2 at normal speed with the timer_elapsed() callback.
 *   3. Initialize the button and apply its initial state through button_changed().
 *   4. On either button edge, button_changed() selects the timer period.
 *   5. On each timer update, timer_elapsed() toggles the LED and logs its state.
 *   6. The main loop remains available for other work.
 */

#include "stm32f4xx.h"
#include "uart.h"
#include "led.h"
#include "button.h"
#include "timer.h"

#define BLINK_NORMAL_MS 1000U
#define BLINK_FAST_MS 100U

/* Application callback: the reusable button module reports press/release state. */
static void button_changed(bool pressed)
{
    TIM2_SetPeriod(pressed ? BLINK_FAST_MS : BLINK_NORMAL_MS);
}

/* Application callback: the timer module has already acknowledged the interrupt. */
static void timer_elapsed(void)
{
    /* Toggle the user LED and report its new state. */
    LED_Toggle();
    USART_write_string(LED_IsOn() ? "LED ON\r\n" : "LED OFF\r\n");
}

int main(void)
{
    /* Configure the LED and UART before enabling the interrupt that uses them. */
    LED_Init();
    USART2_Init();

    /* Start TIM2 before a button callback can change its period. */
    TIM2_Init(BLINK_NORMAL_MS, timer_elapsed);

    /* Apply the initial button state, then enable press/release interrupts. */
    BUTTON_Init(button_changed);

    while (1)
    {
        /* Other work can run here; interrupts handle the button, LED and UART. */
    }
}
