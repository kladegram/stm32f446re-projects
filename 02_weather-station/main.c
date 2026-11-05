/**
 * No-AI challenge - reading docs, some Googling, and writing by hand
 */
#include "stm32f446xx.h"
#include <stdint.h>
#include <stdio.h>

#include "lib/ring_buffer.h"

#include "drivers/SH1106.h"
#include "drivers/aht20.h"
#include "drivers/button.h"
#include "drivers/led.h"
#include "drivers/timer.h"
#include "drivers/usart.h"

volatile bool is_reading_requested = true;

ring_buff_t uart_rx_buffer = {0};

void USART2_IRQHandler(void) {
  char c;

  if (usart_read_char(&c)) {
    // Ignore repeated sensor reading requests
    if (c == 'r' && aht20_is_busy()) {
      return;
    }

    ring_buff_push(&uart_rx_buffer, (uint8_t)c);
  }
}

void EXTI15_10_IRQHandler(void) {
  // Check if user button triggered this interrupt
  if (button_has_interrupt_pending()) {
    led_toggle();
    button_clear_interrupt_flag();

    is_reading_requested = true;
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

      case 'r':
        is_reading_requested = true;

        break;
    }
  }
}

int main(void) {
  // RM0390: 33.16.1 Debug support for low-power modes
  *(volatile uint32_t *)(0xE0042004) |= (1 << 0);

  usart_init();
  led_init();
  button_init();
  timer_init();

  aht20_init((uint8_t)0x38);

  sh1106_init();
  sh1106_clear();

  usart_write_string("\r\n=== Weather Station ===\r\n");

  while (1) {
    // Check not yet processed command from USART
    process_uart_rx_buffer();

    // Activate sensor if reading was requested
    if (is_reading_requested) {
      // Note: At this point, the user may trigger another request, but it can be safely ignored
      aht20_begin(timer_get_ms());
      is_reading_requested = false;
    }

    // Update sensor with current time
    if (aht20_is_busy()) {
      aht20_update(timer_get_ms());
    }

    // Take readings from the sensors if they are ready to be read,
    // and output them using USART and on the screen
    float readings[2];

    if (aht20_take_reading(readings)) {
      usart_write_string("Humidity: ");
      usart_write_float(readings[0]);
      usart_write_string("%\r\n");
      usart_write_string("Temperature: ");
      usart_write_float(readings[1]);
      usart_write_string("C\r\n");

      sh1106_clear();

      h1106_set_pos(2, 10);
      char text_t[24];
      snprintf(text_t, sizeof text_t, "Temp:   %.2fC", readings[1]);
      h1106_write_string((unsigned char *)text_t);

      h1106_set_pos(4, 10);
      char text_h[24];
      snprintf(text_h, sizeof text_h, "Humid.: %.2f%%", readings[0]);
      h1106_write_string((unsigned char *)text_h);
    }

    // Output error message if any
    if (aht20_take_error()) {
      usart_write_string("Error reading data\r\n");

      sh1106_clear();
      h1106_set_pos(2, 10);
      const unsigned char text_e[] = "Error";
      h1106_write_string(text_e);
    }

    // Safely put the CPU into a low-power sleep mode without missing incoming events
    // TODO: Implement "Standby" mode and Wakeup pin
    // TODO: Implement Screen "Entering Sleep Mode" and "Exiting Sleep Mode" sequences
    __disable_irq();

    if (!aht20_is_busy() && !is_reading_requested && ring_buff_is_empty(&uart_rx_buffer)) {
      __WFI();
    }

    __enable_irq();
  }
}
