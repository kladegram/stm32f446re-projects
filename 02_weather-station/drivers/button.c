/**
 * Driver for the NUCLEO-F446RE user button
 *
 * References:
 *
 * UM1724: 7.7 Push-buttons
 *   the user button is connected to the I/O PC13 (pin 2) of the STM32 microcontroller.
 *
 * DS10693: Figure 3. STM32F446xC/E block diagram
 *    GPIO PORT C is connected to AHB1
 *
 * RM0390: Table 1. STM32F446xx register boundary addresses
 *    SYSCFG is connected to APB2
 */
#include "button.h"

// UM1724: 7.7 Push-buttons
#define BTN_PIN 13U

// RM0390: Table 1. STM32F446xx register boundary addresses
#define RCC_BASE 0x40023800

// RM0390: 6.3.10 RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)

// RM0390: Table 1. STM32F446xx register boundary addresses
#define GPIOC_BASE 0x40020800

// RM0390: 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..H)
#define GPIOC_MODER *(volatile uint32_t *)(GPIOC_BASE + 0x00)

// RM0390: 7.4.5 GPIO port input data register (GPIOx_IDR) (x = A..H)
#define GPIOC_IDR *(volatile uint32_t *)(GPIOC_BASE + 0x10)

// RM0390: 6.3.14 RCC APB2 peripheral clock enable register (RCC_APB2ENR)
#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x44)

// RM0390: Table 1. STM32F446xx register boundary addresses
#define SYSCFG_BASE 0x40013800

// RM0390: 8.2.6 SYSCFG external interrupt configuration register 4 (SYSCFG_EXTICR4)
#define SYSCFG_EXTICR4 *(volatile uint32_t *)(SYSCFG_BASE + 0x14)

// RM0390: Table 1. STM32F446xx register boundary addresses
#define EXTI_BASE 0x40013C00

// RM0390: 10.3.1 Interrupt mask register (EXTI_IMR)
#define EXTI_IMR *(volatile uint32_t *)(EXTI_BASE + 0x00)

// RM0390: 10.3.3 Rising trigger selection register (EXTI_RTSR)
#define EXTI_RTSR *(volatile uint32_t *)(EXTI_BASE + 0x08)

// RM0390: 10.3.4 Falling trigger selection register (EXTI_FTSR)
#define EXTI_FTSR *(volatile uint32_t *)(EXTI_BASE + 0x0C)

// RM0390: 10.3.6 Pending register (EXTI_PR)
#define EXTI_PR *(volatile uint32_t *)(EXTI_BASE + 0x14)

void button_init(void) {
  // Enable port C clock; Bit 2 GPIOCEN: IO port C clock enable
  RCC_AHB1ENR |= (1U << 2);

  // Select I/O direction for PC13; 00: Input (reset state)
  GPIOC_MODER &= ~(3U << (BTN_PIN * 2));

  // RM0390: 6.3.14 RCC APB2 peripheral clock enable register (RCC_APB2ENR)
  // Bit 14 SYSCFGEN: System configuration controller clock enable
  RCC_APB2ENR |= (1U << 14);

  /*
    RM0390: 10.2.4 Functional description; Hardware interrupt selection
    ===
    To configure the 23 lines as interrupt sources, use the following procedure:
      • Configure the mask bits of the 23 interrupt lines (EXTI_IMR)
      • Configure the Trigger selection bits of the interrupt lines (EXTI_RTSR and EXTI_FTSR)
      • Configure the enable and mask bits that control the NVIC IRQ channel mapped to the
        external interrupt controller (EXTI) so that an interrupt coming from one of the 23 lines
        can be correctly acknowledged

    RM0390: 8 System configuration controller (SYSCFG)
    ===
    The system configuration controller is mainly used to remap the memory accessible in the
    code area and to manage the external interrupt line connection to the GPIOs.
  */

  // RM0390: 8.2.6 SYSCFG external interrupt configuration register 4 (SYSCFG_EXTICR4)
  // These bits are written by software to select the source input for the EXTIx external interrupt.
  // Bits 15:0 EXTIx[3:0]: EXTI x configuration (x = 12 to 15)
  SYSCFG_EXTICR4 = (SYSCFG_EXTICR4 & ~(15U << 4)) | (2U << 4); // 0010: PC[x] pin

  // Setup the button's EXTI line as an interrupt
  EXTI_IMR |= (1U << BTN_PIN);

  // Disable the 'rising edge' trigger (button release)
  EXTI_RTSR &= ~(1U << BTN_PIN);

  // Enable the 'falling edge' trigger (button press)
  EXTI_FTSR |= (1U << BTN_PIN);

  // Enable EXTI15_10 interrupt, its position is "40"
  // NVIC_ISER1 bits 0 to 31 are for interrupt 32 to 63, respectively
  *(volatile uint32_t *)(0xE000E100 + 0x04 * 1) |= (1U << 8);
}

bool button_is_pressed(void) {
  return (GPIOC_IDR & (1U << BTN_PIN)) == 0U;
}

bool button_has_interrupt_pending(void) {
  return (EXTI_PR & (1U << BTN_PIN));
}

void button_clear_interrupt_flag(void) {
  EXTI_PR = (1U << BTN_PIN);
}