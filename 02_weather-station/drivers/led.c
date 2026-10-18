#include "led.h"

// RM0390: STM32F446xx register boundary addresses
#define RCC_BASE 0x40023800

// RM0390: 6.3.10 RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)

// UM1724; Hardware layout and configuration; user LED connected to PA5
#define LED_PIN 5U

#define GPIOA_BASE 0x40020000

// RM0390: 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..H)
#define GPIOA_MODER *(volatile uint32_t *)(GPIOA_BASE + 0x00)

// RM0390: 7.4.6 GPIO port output data register (GPIOx_ODR) (x = A..H)
#define GPIOA_ODR *(volatile uint32_t *)(GPIOA_BASE + 0x14)

void led_init(void) {
  // Enable port A clock (Bit 0 GPIOAEN: IO port A clock enable)
  RCC_AHB1ENR |= (1U << 0);

  // Select I/O direction for PA5 - 01: General purpose output mode
  GPIOA_MODER &= ~(3U << (LED_PIN * 2));
  GPIOA_MODER |= (1U << (LED_PIN * 2));
}

void led_toggle(void) {
  // Toggle PA5 output level
  GPIOA_ODR ^= (1U << LED_PIN);
}

bool led_is_on(void) {
  return GPIOA_ODR & (1U << LED_PIN);
}
