/*
 * User LED setup and control on the NUCLEO-F446RE.
 *
 * Reference: ST RM0390, RCC and GPIO register descriptions.
 * PA5 drives the green user LED, LD2; a high output turns it on.
 */

#include "stm32f4xx.h"
#include "led.h"

void LED_Init(void)
{
    /*
     * AHB1ENR = AHB1 peripheral clock Enable Register.
     * GPIOAEN = bit 0: enable GPIO port A independently of other modules.
     * Read back the bit to allow the clock enable to take effect.
     */
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
    (void)READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

    /* BSRR bit 21 resets PA5's output bit: start with the LED off. */
    WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BR5);

    /*
     * MODER bits 11:10 = 01: select general purpose output mode for PA5.
     * PA5's reset defaults already give a push-pull, low-speed output without pulls.
     */
    MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
}

void LED_Toggle(void)
{
    /* ODR = Output Data Register, bit 5: reverse PA5's output state. */
    GPIOA->ODR ^= GPIO_ODR_OD5;
}

bool LED_IsOn(void)
{
    /* Read the output state used to drive the active-high LED. */
    return READ_BIT(GPIOA->ODR, GPIO_ODR_OD5) != 0U;
}
