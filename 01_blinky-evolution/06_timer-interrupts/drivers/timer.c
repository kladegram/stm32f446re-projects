/*
 * TIM2 setup, period changes and update interrupts on the STM32F446.
 *
 * Reference: ST RM0390, RCC and general-purpose timer register descriptions.
 * The default 16 MHz HSI clock and APB1 settings give TIM2 a 16 MHz clock.
 * The prescaler makes each count represent 1 ms; ARR selects the update period.
 */

#include "stm32f4xx.h"
#include "timer.h"

/* Application callback; this module has no LED, button or UART dependencies. */
static timer_callback_t update_callback;

void TIM2_Init(uint32_t period_ms, timer_callback_t on_update)
{
    update_callback = on_update;

    /*
     * APB1ENR = APB1 peripheral clock Enable Register.
     * TIM2EN = bit 0: set it to enable TIM2's clock, then read it back.
     */
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
    (void)READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);

    /*
     * PSC = Prescaler register: divides the timer clock by PSC + 1.
     * 16,000,000 Hz / 16,000 = 1,000 Hz: one counter increment per millisecond.
     * ARR = Auto-Reload Register: an update occurs after ARR + 1 counts.
     * For example, ARR = 999 gives a 1000 ms interval.
     */
    WRITE_REG(TIM2->PSC, 16000U - 1U);
    WRITE_REG(TIM2->ARR, period_ms - 1U);

    /*
     * EGR = Event Generation Register.
     * UG = bit 0 (Update Generation): set it to load the prescaler and reset the counter.
     * Clear the resulting status flags so setup does not trigger an interrupt.
     */
    WRITE_REG(TIM2->EGR, TIM_EGR_UG);
    WRITE_REG(TIM2->SR, 0U);

    /*
     * DIER = DMA/Interrupt Enable Register.
     * UIE = bit 0 (Update Interrupt Enable): set it to interrupt on each update event.
     */
    SET_BIT(TIM2->DIER, TIM_DIER_UIE);

    /* Enable external IRQ 28 in the NVIC; startup.s supplies its handler vector. */
    NVIC_EnableIRQ(TIM2_IRQn);

    /*
     * CR1 = Control Register 1.
     * CEN = bit 0 (Counter ENable): set it to start counting.
     */
    SET_BIT(TIM2->CR1, TIM_CR1_CEN);
}

void TIM2_SetPeriod(uint32_t period_ms)
{
    if (READ_REG(TIM2->ARR) != period_ms - 1U)
    {
        /*
         * Stop counting and reset CNT before writing a smaller ARR.
         * ARPE remains disabled from initialization, so ARR takes effect immediately.
         * Clear any old update flag and start a full interval at the new speed.
         */
        CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);
        WRITE_REG(TIM2->CNT, 0U);
        WRITE_REG(TIM2->ARR, period_ms - 1U);
        WRITE_REG(TIM2->SR, ~TIM_SR_UIF);
        SET_BIT(TIM2->CR1, TIM_CR1_CEN);
    }
}

/* Called automatically when TIM2 requests an interrupt. */
void TIM2_IRQHandler(void)
{
    /*
     * SR = Status Register.
     * UIF = bit 0 (Update Interrupt Flag): 1 means an update event is pending.
     */
    if (READ_BIT(TIM2->SR, TIM_SR_UIF) != 0U)
    {
        /*
         * Write zero to UIF to acknowledge this interrupt before running the callback.
         * Leaving UIF set would immediately request the same interrupt again.
         * Writing ones to the other status flags leaves them unchanged.
         */
        WRITE_REG(TIM2->SR, ~TIM_SR_UIF);
        update_callback();
    }
}
