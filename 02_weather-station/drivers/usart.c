#include "usart.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * UM1724: 7.10 USART communication
 *
 * The USART2 interface available on PA2 and PA3 of the STM32 microcontroller
 * can be connected to ST-LINK MCU, ST morpho connector, or ARDUINO® connector.
 *
 */
/**
 * UM1724: 7.13 ARDUINO® connectors
 *
 * Table 19. ARDUINO® connectors on NUCLEO-F446RE
 *
 *    Pin   Pin name    STM32 pin   Function
 *    --------------------------------------
 *    2     D1          PA2         USART2_TX
 *    1     D0          PA3         USART2_RX
 *
 */
/**
 * RM0390: STM32F446xx register boundary addresses
 *
 *    Boundary address              Peripheral    Bus
 *    -------------------------------------------------
 *    0x40004400 - 0x400047FF       USART2        APB1
 *
 */
#define RCC_BASE 0x40023800
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)
#define RCC_APB1ENR *(volatile uint32_t *)(RCC_BASE + 0x40)

#define GPIOA_BASE 0x40020000
#define GPIOA_MODER *(volatile uint32_t *)(GPIOA_BASE + 0x00)
#define GPIOA_AFRL *(volatile uint32_t *)(GPIOA_BASE + 0x20)

#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

#define USART2_BASE 0x40004400
#define USART2_CR1 *(volatile uint32_t *)(USART2_BASE + 0x0C)
#define USART2_CR2 *(volatile uint32_t *)(USART2_BASE + 0x10)
#define USART2_BRR *(volatile uint32_t *)(USART2_BASE + 0x08)
#define USART2_DR *(volatile uint32_t *)(USART2_BASE + 0x04)
#define USART2_SR *(volatile uint32_t *)(USART2_BASE + 0x00)

void usart_init(void) {
  // ^^ Step 1: Enable the clock for the UART GPIOs
  // Enable port A clock (Bit 0 GPIOAEN: IO port A clock enable)
  RCC_AHB1ENR |= (1U << 0);

  // Enable USART2 clock (Bit 17 USART2EN: USART2 clock enable)
  RCC_APB1ENR |= (1U << 17);

  // ^^ Step2: Configure the UART TX/RX pins as alternate function
  /*

    RM0390: 7.3.1 General-purpose I/O (GPIO)
    ===

    During and just after reset, the alternate functions are not active and the
    I/O ports are configured in input floating mode.


    RM0390: 7.3.2 I/O pin multiplexer and mapping
    ===

    Each I/O pin has a multiplexer with sixteen alternate function inputs (AF0
    to AF15) that can be configured through the GPIOx_AFRL (for pin 0 to 7) and
    GPIOx_AFRH (for pin 8 to 15)

    Alternate function
      – Configure the desired I/O as an alternate function in the GPIOx_MODER
      register
      – Select the type, pull-up/pull-down and output speed via the
      GPIOx_OTYPER, GPIOx_PUPDR and GPIOx_OSPEEDER registers, respectively
      – Connect the I/O to the desired AFx in the GPIOx_AFRL or GPIOx_AFRH
      register
*/

  // Select I/O direction mode; 10: Alternate function mode
  GPIOA_MODER = (GPIOA_MODER & ~(3U << (USART2_TX_PIN * 2))) | (2U << (USART2_TX_PIN * 2));
  GPIOA_MODER = (GPIOA_MODER & ~(3U << (USART2_RX_PIN * 2))) | (2U << (USART2_RX_PIN * 2));

  // DS10693: Table 11. Alternate function
  // ===
  // PA2    SART2_TX    AF7
  // PA3    USART2_RX   AF7

  // RM0390: 7.4.9 GPIO alternate function low register (GPIOx_AFRL)
  // ===
  // 0111: AF7
  GPIOA_AFRL = (GPIOA_AFRL & ~(15U << (USART2_TX_PIN * 4))) | (7U << (USART2_TX_PIN * 4));
  GPIOA_AFRL = (GPIOA_AFRL & ~(15U << (USART2_RX_PIN * 4))) | (7U << (USART2_RX_PIN * 4));

  // ^^ Step3: Configure USART
  /*

    RM0390: 25.4.2 Transmitter
    ===

    Procedure:
      1. Enable the USART by writing the UE bit in USART_CR1 register to 1.
      2. Program the M bit in USART_CR1 to define the word length.
      3. Program the number of stop bits in USART_CR2.
      4. Select DMA enable (DMAT) in USART_CR3 if Multi buffer Communication is to
      take place. Configure the DMA register as explained in multibuffer
      communication.
      5. Select the desired baud rate using the USART_BRR register.
      6. Set the TE bit in USART_CR1 to send an idle frame as first transmission.
      7. Write the data to send in the USART_DR register (this clears the TXE bit). Repeat this for
      each data to be transmitted in case of single buffer.

*/
  // 3.1: Enable the USART by writing the UE bit in USART_CR1 register to 1
  USART2_CR1 |= (1U << 13);

  // 3.2: Program the M bit in USART_CR1 to define the word length
  USART2_CR1 &= ~(1U << 12); // 0: 1 Start bit, 8 Data bits, n Stop bit

  // 3.3: Program the number of stop bits in USART_CR2
  USART2_CR2 &= ~(3U << 12); // 00: 1 Stop bit

  // 3.4: Select the desired baud rate using the USART_BRR register
  /*
      16 MHz peripheral clock -> target baud rate 115200
      USARTDIV = 16000000 / (16 * 115200) = 8.6805
      DIV_Mantissa = 8
      DIV_Fraction = 0.6805 * 16 = 10.888 -> 11 (because 11/16 is close enough)
  */
  USART2_BRR = (USART2_BRR & ~(0xFFFF << 0)) | (11U << 0) | (8U << 4);

  // 4: Set the TE bit in USART_CR1 to send an idle frame as first transmission
  USART2_CR1 |= (1U << 3);

  // ^^ Step4: Configure Receiver
  /*
    RM0390: 25.4.3 Receiver
    ===

   When a character is received
    • The RXNE bit is set. It indicates that the content of the shift register is transferred to the
    RDR. In other words, data has been received and can be read (as well as its
    associated error flags).
    • An interrupt is generated if the RXNEIE bit is set.
    • The error flags can be set if a frame error, noise or an overrun error has been detected
    during reception.
    • In multibuffer, RXNE is set after every byte received and is cleared by the DMA read to
    the Data Register.
    • In single buffer mode, clearing the RXNE bit is performed by a software read to the
    USART_DR register. The RXNE flag can also be cleared by writing a zero to it. The
    RXNE bit must be cleared before the end of the reception of the next character to avoid
    an overrun error.
  */

  // RM0390: 25.6.4 Control register 1 (USART_CR1)
  USART2_CR1 |= (1U << 5); // Bit 5 RXNEIE: RXNE interrupt enable

  /**
   * RM0390: 10.2 External interrupt/event controller (EXTI)
   *
   * Vector table for STM32F446xx
   *
   * Position   Priority    Acronym   Address
   * --------------------------------------------
   * 38         45          USART2    0x0000 00D8
   */
  /**
   * PM0214: 4.3.2 Interrupt set-enable register x (NVIC_ISERx)
   *
   * NVIC_ISER1 bits 0 to 31 are for interrupt 32 to 63, respectively
   */
  /**
   * PM0214: 4.3 Nested vectored interrupt controller (NVIC)
   *
   * Table 45. NVIC register summary
   *
   * Address                      Name
   * 0xE000E100 - 0xE000E11F      NVIC_ISER0 - NVIC_ISER7
   */
  // Enable USART2 ("USART2_IRQHandler") interrupt, its position is "38"
  *(volatile uint32_t *)(0xE000E100 + 0x04 * 1) |= (1U << 6);

  // Set the RE bit USART_CR1. This enables the receiver that begins searching for a start bit.
  USART2_CR1 |= (1U << 2);
}

void usart_write_char(char c) {
  /*
  The TXE bit is set by hardware and it indicates:
    • The data has been moved from TDR to the shift register and the data transmission has
    started.
    • The TDR register is empty.
    • The next data can be written in the USART_DR register without overwriting the
    previous data.
  */
  // Bit 7 TXE: Transmit data register empty
  while ((USART2_SR & (1 << 7)) == 0) {
    // Wait for TXE bit to be cleared
  }

  // 5: Write the data to send in the USART_DR register (this clears the TXE bit)
  USART2_DR = (uint8_t)c;
}

void usart_write_string(const char *s) {
  while (*s != '\0') {
    usart_write_char(*s++);
  }
}

void usart_write_int(int i) {
  char t[16];
  snprintf(t, sizeof(t), "%d", i);
  usart_write_string(t);
}

void usart_write_float(float f) {
  char t[16];
  int32_t v = (int32_t)(f * 1000.0f);
  snprintf(t, sizeof(t), "%s%ld.%03ld", f < 0.0f ? "-" : "", labs(v / 1000), labs(v % 1000));
  usart_write_string(t);
}

bool usart_read_char(char *out_char) {
  // Bit 5 RXNE: Read data register not empty
  if (USART2_SR & (1U << 5)) {
    *out_char = (char)USART2_DR;
    return true;
  }

  return false;
}
