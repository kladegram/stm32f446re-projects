/**
 * Part of no-AI challenge, only reading docs and writing by hand
 */

#include <stdint.h>

// RM0390: STM32F446xx register boundary addresses
#define RCC_BASE 0x40023800

// RM0390: 6.3.10 RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)

// * For Green LED
// UM1724; Hardware layout and configuration; user LED connected to PA5
#define LED_PIN 5U

#define GPIOA_BASE 0x40020000

// RM0390: 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..H)
#define GPIOA_MODER *(volatile uint32_t *)(GPIOA_BASE + 0x00)

// RM0390: 7.4.6 GPIO port output data register (GPIOx_ODR) (x = A..H)
#define GPIOA_ODR *(volatile uint32_t *)(GPIOA_BASE + 0x14)

// * For User Button; uses the same AHB1 bus (from RM0390:Memory map) *
// UM1724; Hardware layout and configuration; user button is connected to PC13
#define BTN_PIN 13U

// UM1724; Hardware layout and configuration; user button is connected PC13
#define GPIOC_BASE 0x40020800

// RM0390: 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..H)
#define GPIOC_MODER *(volatile uint32_t *)(GPIOC_BASE + 0x00)

// RM0390: 7.4.5 GPIO port input data register (GPIOx_IDR) (x = A..H)
#define GPIOC_IDR *(volatile uint32_t *)(GPIOC_BASE + 0x10)

void delay(volatile uint32_t c) {
  while (c > 0) {
    c--;
  }
}

int main(void) {
  // Enable port A clock (Bit 0 GPIOAEN: IO port A clock enable)
  RCC_AHB1ENR |= (1U << 0);

  // Enable port C clock (Bit 2 GPIOCEN: IO port C clock enable)
  RCC_AHB1ENR |= (1U << 2);

  // Select I/O direction for PA5 - 01: General purpose output mode
  GPIOA_MODER &= ~(3U << (LED_PIN * 2));
  GPIOA_MODER |= (1U << (LED_PIN * 2));

  // Select I/O direction for PC13 - 00: Input (reset state)
  GPIOC_MODER &= ~(3U << (BTN_PIN * 2));

  while (1) {
    // Toggle PA5 output level
    GPIOA_ODR ^= (1U << LED_PIN);

    // If button is pressed, I/O is LOW
    // https://www.st.com/resource/en/schematic_pack/mb1136-default-c03_schematic.pdf#page=3
    if ((GPIOC_IDR & (1U << BTN_PIN)) == 0U) {
      delay(500000);
    } else {
      delay(1000000);
    }
  }
}
