
/*
 * USART2 debug output and LED toggle on the NUCLEO-F446RE.
 *
 * Reference: ST RM0390, RCC, GPIO and USART register descriptions;
 * STM32F446 datasheet: Alternate function mapping;
 * UM1724 user manual: "USART communication" (USART2 → PA2/PA3 → ST-LINK → USB → computer).
 *
 * Steps:
 *   1. Enable the USART clock.
 *   2. Enable the GPIO clock.
 *   3. Connect the pin to the USART using its alternate function.
 *   4. Configure the communication format and baud rate.
 *   5. Enable USART and its transmitter.
 *   6. Wait for room in the data register, then write a byte.
 */

/*
 * ST's CMSIS header provides register names and bit masks for this chip
 */
#include "stm32f4xx.h"

/* USART2 uses the chip's built-in 16 MHz clock, from the "Embedded bootloader" section */
#define PCLK1_HZ 16000000U

/* Baud rate: the serial terminal must use the same value, its commonly used UART setting */
#define DEBUG_BAUD 115200U

/* PA2 alternate function 7 selects USART2_TX, from the "Alternate function mapping" table. */
#define DEBUG_TX_AF 7U

/* Passed to the busy-wait loop as an iteration count */
#define BLINK_DELAY 1000000U

/*
 * Configure USART2 for transmit-only asynchronous UART operation
 */
void USART2_Init(void)
{
  /*
   * Switch on USART2's clock before using it.
   *
   * RCC = Reset and Clock Control, which manages the chip's clocks.
   * APB1ENR = clock Enable Register for bus APB1, where USART2 is connected.
   * USART2EN = USART2 clock Enable bit: bit 17, with mask 0x00020000 (1U << 17).
   */
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART2EN);

  /*
   * Switch on GPIO port A's clock before configuring or using its pins.
   * This enables the clock for all GPIOA pins, including PA2 (USART2_TX)
   * and PA5 (the onboard LED). Each pin's mode is configured separately.
   *
   * RCC = Reset and Clock Control, which manages the chip's clocks.
   * AHB1ENR = clock Enable Register for bus AHB1, where GPIOA is connected.
   * GPIOAEN = GPIO port A clock Enable bit: bit 0, with mask 0x00000001 (1U << 0).
   */
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

  /*
   * Select USART2's transmit output as PA2's alternate function.
   *
   * GPIOA = GPIO port A, which contains pin PA2.
   * AFR[0] = Alternate Function Register Low (AFRL), for pins PA0..PA7.
   * AFSEL2 = Alternate Function Selection field for pin 2: bits 11:8,
   * with mask 0x00000F00 (0xFU << 8). Each pin's selection uses four bits.
   * DEBUG_TX_AF = 7 (binary 0111): PA2's USART2_TX selection from the datasheet.
   * GPIO_AFRL_AFSEL2_Pos = 8: the starting bit position of PA2's field.
   * PA2's AF7 setting occupies bits 11:8, with register value 0x00000700.
   */
  MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFSEL2,
             DEBUG_TX_AF << GPIO_AFRL_AFSEL2_Pos);

  /*
   * Put PA2 into alternate-function mode so the selected USART can drive it.
   *
   * GPIOA = GPIO port A.
   * MODER = Mode Register, which uses two bits per pin:
   *   00: Input
   *   01: General purpose output
   *   10: Alternate function
   *   11: Analog
   * GPIO_MODER_MODER2 = mode field for pin 2: bits 5:4, with mask 0x00000030 (3U << 4).
   * GPIO_MODER_MODER2_1 = the upper bit of that field: bit 5, with mask 0x00000020 (2U << 4). The suffix _1 means field bit 1.
   *
   * AFR[0] chooses which alternate function; MODER enables that pin mode.
   */
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER2, GPIO_MODER_MODER2_1);

  /*
   * Set the serial communication speed.
   *
   * USART2 = serial communication peripheral number 2.
   * BRR = Baud Rate Register, which sets the clock division for serial timing.
   */
  WRITE_REG(USART2->BRR, (PCLK1_HZ + DEBUG_BAUD / 2U) / DEBUG_BAUD);

  /*
   * Enable USART2 and its transmitter so it can send data.
   *
   * CR1 = Control Register 1, containing USART2's operating switches.
   * TE = Transmitter Enable: bit 3, with mask 0x00000008 (1U << 3).
   * UE = USART Enable: bit 13, with mask 0x00002000 (1U << 13).
   *
   * Both bits must be set to send: UE switches USART2 on, TE enables sending.
   * USART2 also needs its clock enabled, which we did earlier in RCC->APB1ENR.
   * Receiving has a separate RE (Receiver Enable) bit; we leave it off here.
   *
   * The reset settings in CR1, CR2 and CR3 give 8 data bits, no parity,
   * 1 stop bit and no flow control. The receiver stays disabled.
   */
  SET_BIT(USART2->CR1, USART_CR1_TE | USART_CR1_UE);
}

/*
 * Send one byte by polling the transmitter, without using an interrupt.
 */
void USART_write(int ch)
{
  /*
   * Wait until USART2 has room for the next byte.
   *
   * SR = Status Register, which reports what USART2 is currently doing.
   * TXE = Transmit data register Empty: bit 7, with mask 0x00000080 (1U << 7).
   * TXE = 0: the transmit data register still contains a byte.
   * TXE = 1: the register is empty and can accept another byte.
   * TXE can become set while the previous byte is still leaving the serial pin.
   */
  while (READ_BIT(USART2->SR, USART_SR_TXE) == 0U)
  {
    /* The CPU waits here while the USART transfers the previous data. */
  }

  /*
   * Give USART2 the byte to send on PA2.
   *
   * DR = Data Register: writing here supplies the next byte to the transmitter.
   * The function returns after this write; the byte may still be transmitting.
   */
  WRITE_REG(USART2->DR, (uint8_t)ch);
}

/*
 * Simple busy-wait
 */
static void delay(volatile uint32_t count)
{
  while (count--)
  {
    __asm volatile("nop");
  }
}

int main()
{

  /* Enable the clocks and configure PA2 and USART2 before sending data. */
  USART2_Init();

  /*
   * PA5 drives LD2. GPIOA's clock is already enabled by USART2_Init().
   * MODER5 uses bits 11:10; clear the field and write 01 for output mode.
   */
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);

  /* Dummy data to debug */
  uint32_t counter = 0;

  while (1)
  {
    /* GPIOA_ODR bit 5 controls PA5, the green user LED. */
    GPIOA->ODR ^= GPIO_ODR_OD5;

    /* Convert number into ASCII */
    USART_write('0' + counter);
    USART_write('\r');
    USART_write('\n');

    counter = (counter + 1U) % 10U;

    delay(BLINK_DELAY);
  }
}
