/**
 * No-AI challenge, only reading docs and writing by hand
 */

#include <stdint.h>

#include "drivers/button.h"
#include "drivers/led.h"
#include "drivers/usart.h"

void delay(volatile uint32_t c) {
  while (c > 0) {
    c--;
  }
}

int main(void) {
  led_init();
  button_init();
  usart_init();

  while (1) {
    led_toggle();

    if (led_is_on()) {
      usart_write_string("LED is ON\r\n");
    } else {
      usart_write_string("LED is OFF\r\n");
    }

    if (button_is_pressed()) {
      delay(250000);
    } else {
      delay(1000000);
    }
  }
}
