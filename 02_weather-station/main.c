/**
 * No-AI challenge - reading docs, some Googling, and writing by hand
 */
#include "stm32f446xx.h"
#include <stdint.h>

#include "lib/ring_buffer.h"

#include "drivers/aht20.h"
#include "drivers/button.h"
#include "drivers/led.h"
#include "drivers/timer.h"
#include "drivers/usart.h"

ring_buff_t uart_rx_buffer = {0};
bool is_reading_requested = true;

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

      case 'r':
        is_reading_requested = true;

        break;
    }
  }
}

// TODO: infinite loop on missing sensor

int main(void) {
  // RM0390: 33.16.1 Debug support for low-power modes
  *(volatile uint32_t *)(0xE0042004) |= (1 << 0);

  usart_init();
  led_init();
  button_init();
  timer_init();

  timer_start();

  aht20_init((uint8_t)0x38);

  usart_write_string("\r\n=== Weather Station ===\r\n");

  while (1) {
    process_uart_rx_buffer();

    if (is_reading_requested) {
      if (!aht20_is_busy()) {
        aht20_begin(timer_get_ms());
      }

      is_reading_requested = false;
    }

    if (aht20_is_busy()) {
      aht20_update(timer_get_ms());
    }

    float readings[2];

    if (aht20_take_reading(readings)) {
      usart_write_string("Humidity: ");
      usart_write_float(readings[0]);
      usart_write_string("%\r\n");
      usart_write_string("Temperature: ");
      usart_write_float(readings[1]);
      usart_write_string("C\r\n");
    }

    __disable_irq();

    if (!aht20_is_busy() && !is_reading_requested && ring_buff_is_empty(&uart_rx_buffer)) {
      __WFI();
    }

    __enable_irq();
  }
}
