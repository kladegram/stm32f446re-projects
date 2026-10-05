#include <stdint.h>

/*
 * Base addresses
 *
 * Reference: ST RM0390, Section 2.2.2, "Memory map and register boundary
 * addresses", Table 1, "STM32F446xx register boundary addresses".
 */

/* Reset and Clock Control (RCC), AHB1: 0x4002 3800 - 0x4002 3BFF */
#define RCC_BASE 0x40023800UL

/* GPIO port A, AHB1: 0x4002 0000 - 0x4002 03FF */
#define GPIOA_BASE 0x40020000UL

/* GPIO port C, AHB1: 0x4002 0800 - 0x4002 0BFF */
#define GPIOC_BASE 0x40020800UL

/*
 * Register pointers
 *
 * Reference: ST RM0390.
 */

/*
 * RCC AHB1 peripheral clock enable register: enables clocks for GPIO ports.
 * RM0390, Section 6.3.10. Address offset: 0x30.
 */
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))

/*
 * GPIOA mode register: selects input, output, analog, or alternate function.
 * RM0390, Section 7.4.1. Address offset: 0x00.
 */
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

/*
 * GPIOA output data register: writes HIGH or LOW states to the pins.
 * RM0390, Section 7.4.6, GPIOx_ODR (x = A..H). Address offset: 0x14.
 */
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

/*
 * GPIOC mode register: selects input, output, analog, or alternate function.
 * RM0390, Section 7.4.1. Address offset: 0x00.
 */
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00))

/*
 * GPIOC input data register: reads the HIGH or LOW state of the pins.
 * RM0390, Section 7.4.5, GPIOx_IDR (x = A..H). Address offset: 0x10.
 */
#define GPIOC_IDR (*(volatile uint32_t *)(GPIOC_BASE + 0x10))

/* Pin assignments */

/* Green user LED: PA5 (GPIO port A, pin 5). */
#define LED_PIN 5

/* Blue user button: PC13 (GPIO port C, pin 13). */
#define BUTTON_PIN 13

static void delay(volatile uint32_t count)
{
    while (count--)
    {
        __asm volatile("nop");
    }
}

int main(void)
{
    /*
     * Enable GPIOA and GPIOC clocks.
     * RM0390, Section 6.3.28, "RCC register map", Table 21,
     * "RCC register map and reset values".
     *
     * RCC_AHB1ENR:
     *   Bit 0 = GPIOAEN
     *   Bit 2 = GPIOCEN
     */
    RCC_AHB1ENR |= (1U << 0);
    RCC_AHB1ENR |= (1U << 2);

    /*
     * Configure PA5 as an output.
     * RM0390, Section 7.4.1, "GPIO port mode register".
     *
     * MODER uses two bits per pin:
     *
     *   00: Input (reset state)
     *   01: General purpose output mode
     *   10: Alternate function mode
     *   11: Analog mode
     *
     * PA5 uses bits 11:10. First, clear to "00", then write "01".
     */
    GPIOA_MODER &= ~(3U << (LED_PIN * 2));
    GPIOA_MODER |= (1U << (LED_PIN * 2));

    /*
     * Other MODER examples for PA5:
     *
     * Set "10" (alternate function mode):
     *   GPIOA_MODER &= ~(3U << (LED_PIN * 2));
     *   GPIOA_MODER |= (2U << (LED_PIN * 2));
     *
     * Set "11" (analog mode):
     *   GPIOA_MODER |= (3U << (LED_PIN * 2));
     */

    /* Configure PC13 as an input (MODER value: 00). */
    GPIOC_MODER &= ~(3U << (BUTTON_PIN * 2));

    uint32_t blink_delay = 1000000;

    while (1)
    {
        /* Read PC13. */
        if (GPIOC_IDR & (1U << BUTTON_PIN))
        {
            blink_delay = 200000;
        }
        else
        {
            blink_delay = 1000000;
        }

        /* Toggle PA5. */
        GPIOA_ODR ^= (1U << LED_PIN);

        delay(blink_delay);
    }
}
