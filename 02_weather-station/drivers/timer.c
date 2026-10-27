#include "timer.h"

/**
 * RM0390: 2.2.2 Memory map and register boundary addresses
 *
 * Boundary address             Peripheral  Bus
 * 0x4000 0000 - 0x4000 03FF    TIM2        APB1
 */
#define RCC_BASE 0x40023800
#define RCC_APB1ENR *(volatile uint32_t *)(RCC_BASE + 0x40)

#define TIM2_BASE 0x40000000
#define TIM2_CR1 *(volatile uint32_t *)(TIM2_BASE + 0x00)
#define TIM2_DIER *(volatile uint32_t *)(TIM2_BASE + 0x0C)
#define TIM2_SR *(volatile uint32_t *)(TIM2_BASE + 0x10)
#define TIM2_PSC *(volatile uint32_t *)(TIM2_BASE + 0x28)
#define TIM2_EGR *(volatile uint32_t *)(TIM2_BASE + 0x14)
#define TIM2_ARR *(volatile uint32_t *)(TIM2_BASE + 0x2C)
#define TIM2_CNT *(volatile uint32_t *)(TIM2_BASE + 0x24)

void timer_init(void) {
  // ^^ Step1: Enable TIM2 clock (Bit 0 TIM2EN: TIM2 clock enable)
  RCC_APB1ENR |= (1U << 0);

  // ^^ Step2: Set prescaler value
  // Note: `16000 - 1` gives 1 ms ticks, because the timer clock is 16 MHz
  TIM2_PSC = 16000 - 1; // Bits 15:0 PSC[15:0]: Prescaler value

  // ^^ Step3: Set the tick count per loop
  TIM2_ARR = UINT32_MAX;

  // ^^ Step4: Load the prescaler and reset the counter
  TIM2_EGR = (1U << 0); // Bit 0 UG: Update generation

  // ^^ Step5: Clear update flag
  // Avoid generating an update event immediately after intialization
  // https://community.st.com/stm32-mcus-embedded-software-32/why-does-a-timer-operating-in-interrupt-mode-generate-an-update-event-immediately-after-intialization-84019
  timer_clear_update_flag();
}

void timer_start(void) {
  TIM2_CR1 |= (1U << 0); // Bit 0 CEN: Counter enable
}

void timer_clear_update_flag(void) {
  // This bit is set by hardware on an update event. It is cleared by software.
  TIM2_SR = ~(1U << 0); // Bit 0 UIF: Update interrupt flag
}

uint32_t timer_get_ms(void) {
  return TIM2_CNT;
}

// Note: it needs the startup.s entry
void timer_enable_interrupt(void) {
  TIM2_DIER |= (1U << 0); // Bit 0 UIE: Update interrupt enable

  /**
   * RM0390: 10.2 External interrupt/event controller (EXTI)
   *
   * Vector table for STM32F446xx
   *
   * Position   Priority    Acronym   Address
   * --------------------------------------------
   * 28         35          TIM2      0x0000 00B0
   */
  /**
   * PM0214: 4.3.2 Interrupt set-enable register x (NVIC_ISERx)
   *
   * NVIC_ISER0 bits 0 to 31 are for interrupt 0 to 31, respectively
   */
  /**
   * PM0214: 4.3 Nested vectored interrupt controller (NVIC)
   *
   * Table 45. NVIC register summary
   *
   * Address                      Name
   * 0xE000E100 - 0xE000E11F      NVIC_ISER0 - NVIC_ISER7
   */
  // Enable TIM2 ("TIM2_IRQHandler") interrupt, its position is "28"
  *(volatile uint32_t *)(0xE000E100 + 0x04 * 0) |= (1U << 28);

  // Usage example:
  //
  // void TIM2_IRQHandler(void) {
  //   led_toggle();
  //   timer_clear_update_flag();
  // }
}