/*
 * User button setup and interrupts on the NUCLEO-F446RE.
 *
 * Reference: ST RM0390, RCC, GPIO, SYSCFG and EXTI register descriptions;
 * NUCLEO-F446RE board schematic, user button B1 on PC13.
 * The button pulls PC13 low when pressed; the board provides a pull-up.
 */

#include "stm32f4xx.h"
#include "button.h"

/* Application callback; registered before the button interrupt is enabled. */
static button_callback_t state_changed_callback;

void BUTTON_Init(button_callback_t on_change)
{
    state_changed_callback = on_change;

    /*
     * AHB1ENR = AHB1 peripheral clock Enable Register.
     * GPIOCEN = bit 2: enable GPIO port C for the PC13 button pin.
     * Read back the bit to allow the clock enable to take effect.
     */
    SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);
    (void)READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);

    /*
     * APB2ENR = APB2 peripheral clock Enable Register.
     * SYSCFGEN = bit 14: enable the system configuration controller.
     * SYSCFG selects which GPIO port feeds each external interrupt line.
     */
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    (void)READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

    /*
     * Configure only PC13, preserving other GPIOC pins' settings.
     * MODER bits 27:26 = 00: input mode.
     * PUPDR bits 27:26 = 00: use the board's external pull-up.
     */
    CLEAR_BIT(GPIOC->MODER, GPIO_MODER_MODER13);
    CLEAR_BIT(GPIOC->PUPDR, GPIO_PUPDR_PUPD13);

    /*
     * EXTICR[3] = External Interrupt Configuration Register for lines 12..15.
     * EXTI13 uses bits 7:4; value 0010 selects port C, connecting PC13.
     */
    MODIFY_REG(SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI13,
               SYSCFG_EXTICR4_EXTI13_PC);

    /*
     * FTSR = Falling Trigger Selection Register: detect a press (high to low).
     * RTSR = Rising Trigger Selection Register: detect a release (low to high).
     * Bit 13 in each register controls EXTI line 13.
     */
    SET_BIT(EXTI->FTSR, EXTI_FTSR_TR13);
    SET_BIT(EXTI->RTSR, EXTI_RTSR_TR13);

    /* Clear setup events before reporting the initial state and enabling interrupts. */
    WRITE_REG(EXTI->PR, EXTI_PR_PR13);
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

    /* Apply the current state, including a button already held during startup. */
    state_changed_callback(BUTTON_IsPressed());

    /* IMR = Interrupt Mask Register: let EXTI line 13 send requests to the NVIC. */
    SET_BIT(EXTI->IMR, EXTI_IMR_MR13);

    /* NVIC: let IRQ 40 (shared by EXTI lines 10..15) reach the CPU. */
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

bool BUTTON_IsPressed(void)
{
    /* IDR = Input Data Register, bit 13: read the active-low button state. */
    return READ_BIT(GPIOC->IDR, GPIO_IDR_ID13) == 0U;
}

/* EXTI lines 10..15 share this vector; this module handles the button on line 13. */
void EXTI15_10_IRQHandler(void)
{
    if (READ_BIT(EXTI->PR, EXTI_PR_PR13) != 0U)
    {
        /* PR = Pending Register: write ONE to acknowledge only line 13. */
        WRITE_REG(EXTI->PR, EXTI_PR_PR13);

        state_changed_callback(BUTTON_IsPressed());
    }
}
