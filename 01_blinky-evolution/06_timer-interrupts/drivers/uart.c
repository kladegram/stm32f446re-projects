/*
 * USART2 setup and polling transmit on the NUCLEO-F446RE.
 * Adapted from ../../04_uart-debug/main.c.
 *
 * Reference: ST RM0390, RCC, GPIO and USART register descriptions;
 * STM32F446 datasheet, alternate function mapping.
 * PA2 carries USART2_TX to the ST-LINK virtual COM port.
 */

#include "stm32f4xx.h"
#include "uart.h"

/* USART2's APB1 clock with the default 16 MHz HSI clock and bus settings. */
#define PCLK1_HZ 16000000U

/* The computer's serial terminal must use the same baud rate. */
#define DEBUG_BAUD 115200U

/* Alternate function 7 connects PA2 to USART2_TX. */
#define DEBUG_TX_AF 7U

/* Configure transmit-only UART operation using the peripheral reset defaults. */
void USART2_Init(void)
{
  /*
   * APB1ENR = APB1 peripheral clock Enable Register.
   * USART2EN = bit 17: enable USART2's clock before accessing its registers.
   * Read back the bit to allow the clock enable to take effect.
   */
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
  (void)READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);

  /*
   * AHB1ENR = AHB1 peripheral clock Enable Register.
   * GPIOAEN = bit 0: enable GPIO port A for the PA2 transmit pin.
   * USART2 and GPIOA are separate peripherals, so both need clocks.
   */
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
  (void)READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

  /*
   * AFR[0] = Alternate Function Register Low, for pins PA0..PA7.
   * AFSEL2 uses bits 11:8: write 0111 to select USART2_TX on PA2.
   * MODIFY_REG replaces only PA2's field, preserving other pins' settings.
   */
  MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL2,
             DEBUG_TX_AF << GPIO_AFRL_AFSEL2_Pos);

  /* MODER bits 5:4 = 10: enable alternate function mode for PA2. */
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_1);

  /*
   * BRR = Baud Rate Register: sets the clock division for serial timing.
   * With reset-default oversampling by 16, BRR encodes PCLK1 / baud rate.
   * Adding half the baud rate before integer division rounds to the nearest value.
   */
  WRITE_REG(USART2->BRR, (PCLK1_HZ + DEBUG_BAUD / 2U) / DEBUG_BAUD);

  /*
   * CR1 = Control Register 1.
   * TE = bit 3: enable the transmitter.
   * UE = bit 13: enable USART2.
   * Other settings retain their reset defaults: 8 data bits, no parity,
   * 1 stop bit and no flow control. The receiver remains disabled.
   */
  SET_BIT(USART2->CR1, USART_CR1_TE | USART_CR1_UE);
}

/* Send one byte after USART2_Init() has configured the peripheral. */
static void USART_write(int ch)
{
  /*
   * SR = Status Register, bit 7 = TXE: Transmit data register Empty.
   * TXE = 1 means another byte can be written, even if the previous byte
   * is still leaving the serial pin.
   */
  while (READ_BIT(USART2->SR, USART_SR_TXE) == 0U)
  {
    /* The CPU waits here until the transmit data register is available. */
  }

  /*
   * DR = Data Register: supply the next byte to transmit on PA2.
   * The function returns after this write; transmission may still be in progress.
   */
  WRITE_REG(USART2->DR, (uint8_t)ch);
}

/* Send a null-terminated string; the caller supplies any \r\n line ending. */
void USART_write_string(const char *text)
{
  /* Send each character in order, excluding the terminating null byte. */
  while (*text != '\0')
  {
    USART_write(*text++);
  }
}
