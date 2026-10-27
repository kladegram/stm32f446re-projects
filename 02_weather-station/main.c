/**
 * No-AI challenge - reading docs, some Googling, and writing by hand
 */
#include <stdint.h>

#include "lib/ring_buffer.h"

#include "drivers/button.h"
#include "drivers/i2c.h"
#include "drivers/led.h"
#include "drivers/timer.h"
#include "drivers/usart.h"

#define T_ADDRESS (uint8_t)0x38

ring_buff_t uart_rx_buffer = {0};

void USART2_IRQHandler(void) {
  char c;

  if (usart_read_char(&c)) {
    ring_buff_push(&uart_rx_buffer, (uint8_t)c);
  }
}

void EXTI15_10_IRQHandler(void) {
  if (button_has_interrupt_pending()) {
    led_toggle();
    button_clear_interrupt_flag();
  }
}

void process_uart_rx_buffer() {
  uint8_t c;

  if (ring_buff_pop(&uart_rx_buffer, &c)) {
    switch (c) {
      case 't':
        usart_write_int((int)timer_get_ms());
        usart_write_string("\r\n");
        break;

      case 'l':
        usart_write_string((led_is_on() ? "LED is ON" : "LED is OFF"));
        usart_write_string("\r\n");
        break;

        break;
    }
  }
}

enum STATE {
  T_WAIT_FOR_POWER_ON,
  T_TRIGGER_INIT,
  T_WAIT_FOR_INIT,
  T_TRIGGER_MEASUMERENT,
  T_WAIT_FOR_MEASUMERENT,
  T_CHECK_MEASUMERENT_STATUS,
  T_READ_MEASUMERENT_DATA,
  IDLE
};

uint8_t t_init_commands[3] = {0xBE, 0x08, 0x00};
uint8_t t_measure_commands[3] = {0xAC, 0x33, 0x00};

int main(void) {
  usart_init();
  led_init();
  button_init();
  timer_init();
  i2c_init();

  usart_write_string("<<< \r\n");

  timer_start();

  uint32_t previous = timer_get_ms();

  /*
    AHT20 Product manual; Sensor reading process
    ===

    1. Wait 40ms after power-on.

    Before reading the temperature and humidity values, first check whether the calibration
    enable bit Bit [3] of the status word is 1 (you can get a byte of status word by sending 0x71).

    If not 1, need to send 0xbe command (for initialization), this command parameter has
    two bytes, the first byte is 0x08, the second byte is 0x00, and then **wait for 10ms**.

    2. Send the 0xAC command directly (trigger measurement).
    The parameter of this command has two bytes, the first byte
    is 0x33 and the second byte is 0x00.

    3. Wait for 80ms to wait for the measurement to be completed.
    If the read status word Bit [7] is 0, it indicates that the
    measurement is completed, and then six bytes can be read
    in a row; otherwise, continue to wait.

    4. After receiving six bytes, the next byte is the CRC check
    data, the user can read it as needed
  */

  enum STATE state = T_WAIT_FOR_POWER_ON;

  // TODO: infinite loop on missing sensor

  while (1) {
    process_uart_rx_buffer();

    uint32_t now = timer_get_ms();
    uint32_t elapsed = (uint32_t)(now - previous);

    switch (state) {
      case T_WAIT_FOR_POWER_ON: {
        if (elapsed > 40) {
          state = T_TRIGGER_INIT;
        }

      } break;

      case T_TRIGGER_INIT: {
        i2c_write(T_ADDRESS, t_init_commands, 3);

        state = T_WAIT_FOR_INIT;
        previous = now;

      } break;

      case T_WAIT_FOR_INIT: {
        if (elapsed > 10) {
          state = T_TRIGGER_MEASUMERENT;
        }
      } break;

      case T_TRIGGER_MEASUMERENT: {
        i2c_write(T_ADDRESS, t_measure_commands, 3);

        state = T_WAIT_FOR_MEASUMERENT;
        previous = now;

      } break;

      case T_WAIT_FOR_MEASUMERENT: {
        if (elapsed > 80) {
          state = T_CHECK_MEASUMERENT_STATUS;
        }
      } break;

      case T_CHECK_MEASUMERENT_STATUS: {
        uint8_t buffer[1];

        i2c_read(T_ADDRESS, buffer, sizeof buffer);

        // If bit [7] is 0, it indicates that the measurement is completed
        if ((buffer[0] & (1U << 7)) == 0) {
          state = T_READ_MEASUMERENT_DATA;
        }

      } break;

      case T_READ_MEASUMERENT_DATA: {
        uint8_t buffer[6];

        i2c_read(T_ADDRESS, buffer, sizeof buffer);

        // Temperature data are located in the 3rd (partially), 4th, and 5th bytes of the reading
        uint32_t data = ((buffer[3] & 0b00001111) << 16) | (buffer[4] << 8) | buffer[5];

        usart_write_string("temperature: ");
        usart_write_float((float)(((data / 1048576.0f) * 200) - 50));
        usart_write_string("C\r\n");

        state = IDLE;
      } break;

      case IDLE: {
        // wait
      } break;
    }
  }
}
