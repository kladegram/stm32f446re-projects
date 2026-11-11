#include "./spi.h"

/*
    Reference: 1.5 Pin Definition
    https://cdn.sparkfun.com/assets/2/6/8/9/7/1.3inch-SH1106-OLED_Datasheet.pdf

    RES# - Power Reset for Controller and Driver
    ===
    This pin is reset signal input. When the pin is low, initialization of the chip is
    executed. Keep this pin pull high during normal operation.

    D/C# - Data/Command Control
    ===
    When the pin is pulled high and serial interface mode is selected, the data at SDIN
    will be interpreted as data. When it is pulled low, the data at SDIN will be
    transferred to the command register.

    CS# - Chip Select
    ===
    This pin is the chip select input. The chip is enabled for MCU communication only
    when CS# is pulled low.
*/
/*
    DS10693: Table 11. Alternate function
    ===
    PB13    SPI2_SCK    AF5
    PB15    SPI2_MOSI   AF5
*/
/*
    DS10693: Table 12. STM32F446xC/E register boundary addresses
    ===
    Bus         Boundary address                Peripheral
    ---------------------------------------------------------
    APB1        0x4000 3800 - 0x4000 3BFF       SPI2 / I2S2
*/
#define RCC_BASE 0x40023800
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOB_BASE 0x40020400
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_AFRL (*(volatile uint32_t *)(GPIOB_BASE + 0x20))
#define GPIOB_AFRH (*(volatile uint32_t *)(GPIOB_BASE + 0x24))

#define SPI2_BASE 0x40003800
#define SPI2_CR1 (*(volatile uint32_t *)(SPI2_BASE + 0x00))
#define SPI2_SR (*(volatile uint32_t *)(SPI2_BASE + 0x08))
#define SPI2_DR (*(volatile uint32_t *)(SPI2_BASE + 0x0C))

#define SPI_TIMEOUT_MS 20U

static spi2_config_t spi2_config;

/*
  Checks flags with timeout, replaces
      `while (!(I2C_SR1 & (1U << 0))) {};`
  with
      if (i2c_wait_for(&I2C_SR1, (1U << 0), true)) {}
*/
static bool spi_wait_for(volatile uint32_t *reg, uint32_t mask, bool wanted_set) {
  return timer_wait_for(reg, mask, wanted_set, SPI_TIMEOUT_MS);
}

void spi2_init(const spi2_config_t *config) {
  spi2_config = *config;

  /*
    RM0390: 26.3.7 SPI configuration
    ===

    1. Write proper GPIO registers:
        Configure GPIO for MOSI, MISO and SCK pins.

    2. Write to the SPI_CR1 register:
        a) Configure the serial clock baud rate using the BR[2:0] bits (Note: 3).
        b) Configure the CPOL and CPHA bits combination to define one of the four
        relationships between the data transfer and the serial clock. (Note: 2 - except the
        case when CRC is enabled at TI mode).
        c) Select simplex or half-duplex mode by configuring RXONLY or BIDIMODE and
        BIDIOE (RXONLY and BIDIMODE can't be set at the same time).
        d) Configure the LSBFIRST bit to define the frame format (Note: 2).
        e) Configure the CRCEN and CRCEN bits if CRC is needed (while SCK clock signal
        is at idle state).
        f) Configure SSM and SSI (Note: 2).
        g) Configure the MSTR bit (in multimaster NSS configuration, avoid conflict state on
        NSS if master is configured to prevent MODF error).
        h) Set the DFF bit to configure the data frame format (8 or 16 bits).
  */
  //^^ Step 1. Write proper GPIO registers
  // Enable port B clock
  RCC_AHB1ENR |= (1U << 1);

  // Enable SPI2 clock
  RCC_APB1ENR |= (1U << 14);

  // Set pin modes; SCK and MOSI pins - AF5; "10": Alternate function mode
  GPIOB_MODER =
      (GPIOB_MODER & ~(3U << (spi2_config.sck_pin * 2))) | (2U << (spi2_config.sck_pin * 2));
  GPIOB_MODER =
      (GPIOB_MODER & ~(3U << (spi2_config.mosi_pin * 2))) | (2U << (spi2_config.mosi_pin * 2));

  // Set pin alternate functions
  //    - GPIOx_AFRL (for pin 0 to 7) and GPIOx_AFRH (for pin 8 to 15)
  //    - "0101": AF5
  uint8_t sck_pin = spi2_config.sck_pin;
  uint8_t mosi_pin = spi2_config.mosi_pin;

  if (sck_pin >= 8) {
    GPIOB_AFRH =
        (GPIOB_AFRH & ~(0b1111U << ((sck_pin - 8) * 4))) | (0b0101U << ((sck_pin - 8) * 4));
  } else {
    GPIOB_AFRL = (GPIOB_AFRL & ~(0b1111U << (sck_pin * 4))) | (0b0101U << (sck_pin * 4));
  }

  if (mosi_pin >= 8) {
    GPIOB_AFRH =
        (GPIOB_AFRH & ~(0b1111U << ((mosi_pin - 8) * 4))) | (0b0101U << ((mosi_pin - 8) * 4));
  } else {
    GPIOB_AFRL = (GPIOB_AFRL & ~(0b1111U << (mosi_pin * 4))) | (0b0101U << (mosi_pin * 4));
  }

  //^^ Step2: Write to the SPI_CR1 register:
  // Bit 15 BIDIMODE: Bidirectional data mode enable
  // This bit enables half-duplex communication using common single bidirectional data line.
  // Keep RXONLY bit clear when bidirectional mode is active.
  //    0: 2-line unidirectional data mode selected
  //    1: 1-line bidirectional data mode selected

  // Bit 14 BIDIOE: Output enable in bidirectional mode
  // This bit combined with the BIDIMODE bit selects the direction of transfer in bidirectional
  // mode
  //    0: Output disabled (receive-only mode)
  //    1: Output enabled (transmit-only mode)

  // Bit 9 SSM: Software slave management
  // When the SSM bit is set, the NSS pin input is replaced with the value from the SSI bit.
  //    0: Software slave management disabled
  //    1: Software slave management enabled

  // Bit 8 SSI: Internal slave select
  //    This bit has an effect only when the SSM bit is set. The value of this bit is forced onto
  //    the NSS pin and the IO value of the NSS pin is ignored.

  // Bits 5:3 BR[2:0]: Baud rate control
  //    011: fPCLK/16

  // Bit 2 MSTR: Master selection
  //    0: Slave configuration
  //    1: Master configuration

  SPI2_CR1 = (1U << 2)                               // MSTR: Master
             | (uint32_t)(spi2_config.baud_div << 3) // BR (3: fPCLK/16)
             | (1U << 8)                             // SSI: Internal NSS high
             | (1U << 9)   // SSM: NSS pin input is replaced with the value from the SSI bit
             | (1U << 15)  // BIDIMODE: 1-line
             | (1U << 14); // BIDIOE: transmit-only mode

  //^^ Enable SPI2
  SPI2_CR1 |= (1U << 6);
}

bool spi2_write(unsigned char c) {
  /*
    RM0390: 26.3.9 Data transmission and reception procedures; Tx buffer handling
    ===
    The TXE flag (Tx buffer empty) is set when the data are transferred from the Tx buffer to the
    shift register. It indicates that the internal Tx buffer is ready to be loaded with the next
    data.
  */
  // Wait until TXE bit is set
  if (!spi_wait_for(&SPI2_SR, (1U << 1), true)) {
    // Bit 1 TXE: Transmit buffer empty
    //  0: Tx buffer not empty
    //  1: Tx buffer empty
    return false;
  }

  SPI2_DR = c; // Send data to SPI Data register

  /*
    RM0390: 26.3.9 Data transmission and reception procedures; Sequence handling
    ===
    The BSY bit is set when a current data frame transaction is ongoing. When the clock signal
    runs continuously, the BSY flag remains set between data frames on the master side.

    For some configurations, the BSY flag can be used during the last data transfer to wait until
    the completion of the transfer.

    RM0390: 26.3.10 Procedure for disabling the SPI
    ===
    During discontinuous communications, there is a 2 APB clock period delay between the
    write operation to the SPI_DR register and BSY bit setting. As a consequence it is
    mandatory to wait first until TXE is set and then until BSY is cleared after writing the last
    data.
  */
  // Wait until TXE bit is set
  if (!spi_wait_for(&SPI2_SR, (1U << 1), true)) {
    // Bit 1 TXE: Transmit buffer empty
    //  0: Tx buffer not empty
    //  1: Tx buffer empty
    return false;
  }

  // Wait until BSY bit is cleared
  if (!spi_wait_for(&SPI2_SR, (1U << 7), false)) {
    // Bit 7 BSY: Busy flag
    //  0: SPI (or I2S) not busy
    //  1: SPI (or I2S) is busy in communication or Tx buffer is not empty
    return false;
  }

  return true;
}