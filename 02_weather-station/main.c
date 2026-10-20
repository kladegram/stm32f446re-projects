/**
 * No-AI challenge, only reading docs and writing by hand
 */
#include <stdint.h>

#include "drivers/button.h"
#include "drivers/led.h"
#include "drivers/timer.h"
#include "drivers/usart.h"

// void log_time(void) {
//   usart_write_int((int)timer_get_time());
//   usart_write_string("\r\n");
// }

void EXTI15_10_IRQHandler(void) {
  if (button_has_interrupt_pending()) {
    usart_write_string(">> button press\r\n");
    button_clear_update_flag();
    led_toggle();
  }
}

int main(void) {
  led_init();
  button_init();
  usart_init();

  usart_write_string("<<< \r\n");

  timer_init();
  timer_start();

  uint32_t previous = timer_get_ms();

  while (1) {
    uint32_t now = timer_get_ms();
    uint32_t elapsed = (uint32_t)(now - previous);

    uint32_t delay = button_is_pressed() ? 125 : 500;

    if (elapsed >= delay) {
      previous = now;

      // usart_write_string("led_toggle \r\n");
      // led_toggle();
    }
  }
}
