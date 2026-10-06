#include <stdint.h>

/* GPIO registers: see ../02_baremetal/main.c for details. */
#define RCC_BASE 0x40023800UL
#define GPIOA_BASE 0x40020000UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

/*
 * SysTick is a 24-bit down-counter built into the Cortex-M4 core.
 * Its registers use fixed core addresses, independent of the GPIO/RCC buses.
 *
 * Reference: ST PM0214, "SysTick timer"
 */

/* Control/status register: enables the timer, interrupt, and clock source. */
#define SYST_CSR (*(volatile uint32_t *)0xE000E010)

/* Reload value register: sets the period to RELOAD + 1 clock cycles. */
#define SYST_RVR (*(volatile uint32_t *)0xE000E014)

/* Current value register: writing any value clears the counter and COUNTFLAG. */
#define SYST_CVR (*(volatile uint32_t *)0xE000E018)

/*
 * Elapsed milliseconds, updated by the interrupt and read by main()
 */
static volatile uint32_t milliseconds = 0;

static void gpio_init(void)
{
  /* Enable GPIOA and configure PA5 as an output. */
  RCC_AHB1ENR |= (1U << 0);

  GPIOA_MODER &= ~(3U << (5 * 2));
  GPIOA_MODER |= (1U << (5 * 2));
}

/*
 * Called automatically every 1 ms by the SysTick exception.
 * startup.s connects this function to exception 15 in the vector table.
 */
void SysTick_Handler(void)
{
  milliseconds++;
}

static void systick_init(void)
{
  /*
   * The CPU uses the default 16 MHz HSI clock after reset.
   * 16,000,000 cycles/s / 1,000 ticks/s = 16,000 cycles per millisecond.
   * Using 16,000 - 1 because the period is RELOAD + 1 cycles.
   */
  SYST_RVR = 16000 - 1;

  /* Clear the current count before starting the timer. */
  SYST_CVR = 0;

  /*
   * SYST_CSR:
   *   Bit 0 = ENABLE: start the counter.
   *   Bit 1 = TICKINT: request an interrupt when the counter reaches zero.
   *   Bit 2 = CLKSOURCE: use the CPU clock.
   */
  SYST_CSR =
      (1U << 0) |
      (1U << 1) |
      (1U << 2);
}

int main(void)
{
  gpio_init();
  systick_init();

  uint32_t previous = 0;

  while (1)
  {
    uint32_t now = milliseconds;

    /*
     * Toggle every 500 ms, unsigned subtraction also handles the counter wrap around
     */
    if ((uint32_t)(now - previous) >= 500)
    {
      previous = now;

      GPIOA_ODR ^= (1U << 5);
    }

    /* Rest of ode */
  }
}
