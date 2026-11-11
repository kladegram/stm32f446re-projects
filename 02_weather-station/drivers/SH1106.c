
#include "./SH1106.h"

/*
  Reference:
  https://cdn.sparkfun.com/assets/2/6/8/9/7/1.3inch-SH1106-OLED_Datasheet.pdf


  RES#
  ===
  This pin is reset signal input. When the pin is low, initialization of the chip is
  executed. Keep this pin pull high during normal operation.


  CS#
  ===
  This pin is the chip select input. The chip is enabled for MCU communication only when CS# is
  pulled low.


  D/C#
  ===
  This pin is Data/Command control pin. When the pin is pulled high, the input at D7~D0 is treated
  as display data. When the pin is pulled low, the input at D7~D0 will be transferred to the command
  register.


  Write instruction/command sample
  ===

  ```
  void write_i(unsigned char ins)
  {
    DC=0;
    CS=0;
    WR=1;
    P1=ins;
    WR=0;
    WR=1;
    CS=1;
  }
  ```

  Write data sample
  ===

  ```
  void write_d(unsigned char dat)
  {
    DC=1;
    CS=0;
    WR=1;
    P1=dat;
    WR=0;
    WR=1;
    CS=1;
  }
  ```

*/
#define GPIOB_BASE 0x40020400
#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_BSRR (*(volatile uint32_t *)(GPIOB_BASE + 0x18U))

#define GPIO_RESET_MASK(pin) (1U << ((pin) + 16U))
#define GPIO_SET_MASK(pin) (1U << (pin))

static sh1106_config_t sh1106_config;

static void sh1106_set_res(void) {
  GPIOB_BSRR = GPIO_SET_MASK(sh1106_config.res_pin);
}

static void sh1106_reset_res(void) {
  GPIOB_BSRR = GPIO_RESET_MASK(sh1106_config.res_pin);
}

static void sh1106_set_dc(void) {
  GPIOB_BSRR = GPIO_SET_MASK(sh1106_config.dc_pin);
}

static void sh1106_reset_dc(void) {
  GPIOB_BSRR = GPIO_RESET_MASK(sh1106_config.dc_pin);
}

static void sh1106_set_cs(void) {
  GPIOB_BSRR = GPIO_SET_MASK(sh1106_config.cs_pin);
}

static void sh1106_reset_cs(void) {
  GPIOB_BSRR = GPIO_RESET_MASK(sh1106_config.cs_pin);
}

void sh1106_init(const sh1106_config_t *config) {
  sh1106_config = *config;

  /*

    Reference: https://cdn.sparkfun.com/assets/2/6/8/9/7/1.3inch-SH1106-OLED_Datasheet.pdf
    ===

    ```
    RES=1;
    delay(1000);
    RES=0;
    delay(1000);
    RES=1;
    delay(1000);
    ```
  */
  // Set pin mode "01": General purpose output mode
  GPIOB_MODER =
      (GPIOB_MODER & ~(3U << (sh1106_config.dc_pin * 2))) | (1U << (sh1106_config.dc_pin * 2));
  GPIOB_MODER =
      (GPIOB_MODER & ~(3U << (sh1106_config.cs_pin * 2))) | (1U << (sh1106_config.cs_pin * 2));
  GPIOB_MODER =
      (GPIOB_MODER & ~(3U << (sh1106_config.res_pin * 2))) | (1U << (sh1106_config.res_pin * 2));

  sh1106_set_res();
  timer_delay_ms(1);
  sh1106_reset_res();
  timer_delay_ms(1);
  sh1106_set_res();
  timer_delay_ms(1);

  sh1106_write_command(0xAE); /*display off*/

  sh1106_write_command(0x02); /*set lower column address*/
  sh1106_write_command(0x10); /*set higher column address*/
  sh1106_write_command(0x40); /*set display start line*/

  sh1106_write_command(0xB0); /*set page address*/

  sh1106_write_command(0x81); /*contract control*/
  sh1106_write_command(0xff); /*128*/

  sh1106_write_command(0xA1); /*set segment remap*/

  sh1106_write_command(0xA6); /*normal / reverse*/

  sh1106_write_command(0xA8); /*multiplex ratio*/
  sh1106_write_command(0x3F); /*duty = 1/64*/

  sh1106_write_command(0xad); /*set charge pump enable*/
  sh1106_write_command(0x8b); /* 0x8B 内供 VCC */

  sh1106_write_command(0x33); /*0X30---0X33 set VPP 9V */

  sh1106_write_command(0xC8); /*Com scan direction*/

  sh1106_write_command(0xD3); /*set display offset*/
  sh1106_write_command(0x00); /* 0x20 */

  sh1106_write_command(0xD5); /*set osc division*/
  sh1106_write_command(0x80);

  sh1106_write_command(0xD9); /*set pre-charge period*/
  sh1106_write_command(0x1f); /*0x22*/

  sh1106_write_command(0xDA); /*set COM pins*/
  sh1106_write_command(0x12);

  sh1106_write_command(0xdb); /*set vcomh*/
  sh1106_write_command(0x40);

  sh1106_write_command(0xAF); /*display ON*/
}

bool sh1106_write_command(unsigned char command) {
  sh1106_reset_dc();
  sh1106_reset_cs();
  bool rez = spi2_write(command);
  sh1106_set_cs();

  return rez;
}

bool sh1106_write_data(unsigned char data) {
  sh1106_set_dc();
  sh1106_reset_cs();
  bool rez = spi2_write(data);
  sh1106_set_cs();

  return rez;
}

/*

Display Data RAM
===
The size of the RAM is 132 X 64 bits.


Set Column Address
===

Set Lower Column Address: (OOH - OFH)
Set Higher Column Address: (10H -1FH)

Specifies column address of display RAM. Divide the column address into 4 higher bits and 4 lower
bits. Set each of them into successions. When the microprocessor repeats to access to the display
RAM, the column address counter is incremented during each access until address 131 is accessed. The
page address is not changed during this time


Set Page Address
===

Set Page Address: (BOH - B7H)
Specifies page address to load display RAM data to page address register. Any RAM data bit can be
accessed when its page address and column address are specified. The display remains unchanged even
when the page address is changed.


Write Display Data
===

Write 8-bit data in display RAM. As the column address is incremental by 1 automatically after each
write, the microprocessor can continue to write data of multiple words.

*/

bool h1106_write_string(const unsigned char *s) {
  while (*s != '\0') {
    unsigned char c = (unsigned char)*s++;
    for (unsigned i = 0; i < 8; i++) {
      if (!sh1106_write_data(Font_8x8[c][i])) {
        return false;
      }
    }
  }

  return true;
}

bool h1106_set_pos(uint32_t page, uint32_t column) {
  // Get page_address, it starts from BOH
  uint8_t page_address = 0xB0 + page;
  // Extract the lower 4 bits of the column address
  uint8_t lower_column_address = column & 0b00001111;
  // Extract the higher 4 bits of the column address; starts from 10H
  uint8_t higher_column_address = 0x10U + ((column >> 4) & 0b00001111);

  if (!sh1106_write_command(page_address) ||         //
      !sh1106_write_command(lower_column_address) || //
      !sh1106_write_command(higher_column_address)   //
  ) {
    return false;
  }

  return true;
}

bool sh1106_clear(void) {
  for (uint8_t page = 0; page < 8; page++) {
    if (!h1106_set_pos(page, 0)) {
      return false;
    }

    for (uint16_t column = 0; column < 132; column++) {
      if (!sh1106_write_data(0x00)) {
        return false;
      }
    }
  }

  return true;
}

bool sh1106_enter_sleep(void) {
  // Set Display Off
  if (!sh1106_write_command(0xAE)) {
    return false;
  }

  // Set Charge Pump
  return sh1106_write_command(0xAD) && sh1106_write_command(0x8A);
}

bool sh1106_exit_sleep(void) {
  // Set Charge Pump
  if (!sh1106_write_command(0xAD) || !sh1106_write_command(0x8B)) {
    return false;
  }

  // Set Display On
  if (!sh1106_write_command(0xAF)) {
    return false;
  }

  // Power Stabilized (100ms Delay Recommended)
  timer_delay_ms(100);

  return true;
}