/**
 * NO-AI challenge - reading docs, some Googling, and writing by hand
 */
/*

  DS10693: Table 10. STM32F446xx pin and ball descriptions
  UM1724: Table 29. ST morpho connector on NUCLEO-F401RE, NUCLEO-F411RE, NUCLEO-F446RE
  ===

  Name        ST morpho connector     Function                        Display Pin
  ------------------------------------------------------------------------------
  GND         CN7  / 20               -                           ->  Gnd
  +3.3V       CN7  / 16               -                           ->  Vcc
  PB13        CN10 / 30               SPI2_SCK                    ->  Clk
  PB15        CN10 / 26               SPI2_MOSI                   ->  Mosi
  PB2         CN10 / 22               General-purpose I/O         ->  Res
  PB1         CN10 / 24               General-purpose I/O         ->  Dc
  PB14        CN10 / 28               General-purpose I/O         ->  Cs

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

void USART2_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void process_uart_rx_buffer(void);
void display_reading(float readings[2]);
void display_error(void);

enum app_states {
  //
  APP_INIT,
  APP_MEASURING,
  APP_SHOWING,
  APP_ENTER_IDLE,
  APP_IDLE
};

struct app_context {
  enum app_states state;
  uint32_t display_started_ms;
  volatile bool reading_requested;
  ring_buff_t uart_rx_buffer;
};

static struct app_context app = {
    .state = APP_INIT,         //
    .reading_requested = true, //
};

static const spi2_config_t spi = {
    .sck_pin = 13U,  // PB13
    .mosi_pin = 15U, // PB15
    .baud_div = 3U   // fPCLK / 16
};

static const sh1106_config_t display = {
    .res_pin = 2U, // PB2
    .dc_pin = 1U,  // PB1
    .cs_pin = 14U  // PB14
};

int main(void) {
  // RM0390: 33.16.1 Debug support for low-power modes
  *(volatile uint32_t *)(0xE0042004) |= (1 << 0);

  usart_init();
  led_init();
  button_init();
  timer_init();

  aht20_init((uint8_t)0x38);

  spi2_init(&spi);
  sh1106_init(&display);
  sh1106_clear();

  usart_write_string("\r\n=== Weather Station ===\r\n");

  while (1) {
    // Check not yet processed command from USART
    process_uart_rx_buffer();

    // Activate sensor if reading was requested
    if (app.reading_requested) {
      if (app.state == APP_IDLE) {
        sh1106_exit_sleep();
      }

      if (app.state != APP_MEASURING) {
        aht20_begin(timer_get_ms());

        app.state = APP_MEASURING;
      }

      app.reading_requested = false;
    }

    switch (app.state) {
      case APP_MEASURING:
        // Update sensor with current time
        aht20_update(timer_get_ms());

        // Take readings from the sensors if they are ready to be read,
        // and output them using USART and on the screen
        float readings[2];

        if (aht20_take_reading(readings)) {
          display_reading(readings);

          app.display_started_ms = timer_get_ms();
          app.state = APP_SHOWING;
        }

        // Output error message if any
        if (aht20_take_error()) {
          display_error();

          app.display_started_ms = timer_get_ms();
          app.state = APP_SHOWING;
        }

        break;

      case APP_SHOWING:
        // Wait for a delay before turning off the display
        if ((uint32_t)(timer_get_ms() - app.display_started_ms) > 5000U) {
          app.state = APP_ENTER_IDLE;
        }

        break;

      case APP_ENTER_IDLE:
        // Turn off the display
        sh1106_enter_sleep();
        sh1106_clear();

        app.state = APP_IDLE;

        break;

      case APP_IDLE:
        // Safely put the CPU into a low-power sleep mode without missing incoming events
        __disable_irq();

        if (ring_buff_is_empty(&app.uart_rx_buffer) && !app.reading_requested) {
          __WFI();
        }

        __enable_irq();

        break;

      default:
        break;
    }
  }
}

void USART2_IRQHandler(void) {
  char c;

  if (usart_read_char(&c)) {
    // Ignore repeated sensor reading requests
    if (c == 'r' && aht20_is_busy()) {
      return;
    }

    ring_buff_push(&app.uart_rx_buffer, (uint8_t)c);
  }
}

void EXTI15_10_IRQHandler(void) {
  // Check if user button triggered this interrupt
  if (button_has_interrupt_pending()) {
    led_toggle();
    button_clear_interrupt_flag();

    app.reading_requested = true;
  }
}

void process_uart_rx_buffer(void) {
  uint8_t c;

  if (ring_buff_pop(&app.uart_rx_buffer, &c)) {
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
        app.reading_requested = true;

        break;
    }
  }
}

void display_reading(float readings[2]) {
  //^^ USART
  usart_write_string("Humidity: ");
  usart_write_float(readings[0]);
  usart_write_string("%\r\n");
  usart_write_string("Temperature: ");
  usart_write_float(readings[1]);
  usart_write_string("C\r\n");

  //^^ Display
  h1106_set_pos(2, 10);
  char text_t[24];
  snprintf(text_t, sizeof text_t, "Temp.: %6.2fC", readings[1]);
  h1106_write_string((unsigned char *)text_t);

  h1106_set_pos(4, 10);
  char text_h[24];
  snprintf(text_h, sizeof text_h, "Humid.:%6.2f%%", readings[0]);
  h1106_write_string((unsigned char *)text_h);
}

void display_error(void) {
  //^^ USART
  usart_write_string("Error reading data\r\n");

  //^^ Display
  sh1106_clear();
  h1106_set_pos(2, 10);
  const unsigned char text_e[] = "Error";
  h1106_write_string(text_e);
}