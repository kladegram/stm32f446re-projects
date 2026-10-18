#include "button.h"

// * Uses AHB1 bus (from RM0390:Memory map)

// RM0390: STM32F446xx register boundary addresses
#define RCC_BASE 0x40023800

// RM0390: 6.3.10 RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)

// UM1724; Hardware layout and configuration; user button is connected to PC13
#define BTN_PIN 13U

// UM1724; Hardware layout and configuration; user button is connected PC13
#define GPIOC_BASE 0x40020800

// RM0390: 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..H)
#define GPIOC_MODER *(volatile uint32_t *)(GPIOC_BASE + 0x00)

// RM0390: 7.4.5 GPIO port input data register (GPIOx_IDR) (x = A..H)
#define GPIOC_IDR *(volatile uint32_t *)(GPIOC_BASE + 0x10)

void button_init(void) {
  // Enable port C clock (Bit 2 GPIOCEN: IO port C clock enable)
  RCC_AHB1ENR |= (1U << 2);

  // Select I/O direction for PC13 - 00: Input (reset state)
  GPIOC_MODER &= ~(3U << (BTN_PIN * 2));
}

bool button_is_pressed(void) {
  return (GPIOC_IDR & (1U << BTN_PIN)) == 0U;
}
