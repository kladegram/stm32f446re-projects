/*
 * FreeRTOS tasks, user LED blink, and UART logging on the NUCLEO-F446RE.
 * Inspired by https://www.youtube.com/watch?v=i_eU16X67qU
 * Follows stage 7 of the reference: tasks send messages through a queue,
 * and task_log sends them through UART, preventing messages from mixing.
 *
 * Steps:
 *   1. Configure USART2 on PA2 and the user LED on PA5.
 *   2. Create a log queue and the LED, data, and logging tasks.
 *   3. Start the FreeRTOS scheduler.
 *   4. Toggle PA5 every 500 ms and queue its state for logging.
 *   5. Queue a status message every 3 seconds.
 *   6. Send queued messages through UART from the logging task.
 */

/* ST's CMSIS header provides register names and bit masks for this chip. */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"

#define LED_PIN GPIO_ODR_OD5
#define BLINK_PERIOD_MS 500U
#define DATA_PERIOD_MS 3000U

static QueueHandle_t log_queue;

static void init(void)
{
  /*
   * Set up the interrupt table, clock information, and interrupt priorities.
   * These are optional for normal flash boot with the default clock/priorities.
   * - VTOR and barriers: when relocating vectors, e.g. with a bootloader or RAM.
   *   Set VTOR to the actual vector table address.
   * - SystemCoreClockUpdate: after changing the CPU clock.
   * - Priority grouping: if other code changes the grouping needed by FreeRTOS.
   */
  /* Use the interrupt vector table in flash. */
  SCB->VTOR = FLASH_BASE;

  /* Wait for preceding memory accesses to finish. */
  __DSB();

  /* Refresh the CPU instruction pipeline. */
  __ISB();

  /* Update the CPU clock value used by FreeRTOS. */
  SystemCoreClockUpdate();

  /* Use all interrupt priority bits for preemption. */
  NVIC_SetPriorityGrouping(0U);

  /* Also enables GPIOA's clock for both PA2 (UART) and PA5 (LED). */
  USART2_Init();

  /* Start with the LED off, then set PA5 to output mode (01). */
  WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BR5);
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER5, GPIO_MODER_MODER5_0);
}

/* Queue stores a pointer; string literals remain valid throughout execution. */
static void log_write(const char *message)
{
  xQueueSend(log_queue, &message, 0);
}

static void task_led(void *argument)
{
  (void)argument;

  while (1)
  {
    GPIOA->ODR ^= LED_PIN;

    log_write((GPIOA->ODR & LED_PIN) ? "led: on\r\n" : "led: off\r\n");

    /* This task waits; the scheduler can run other tasks during the delay. */
    vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD_MS));
  }
}

static void task_data(void *argument)
{
  (void)argument;

  while (1)
  {
    log_write("data: FreeRTOS running\r\n");
    vTaskDelay(pdMS_TO_TICKS(DATA_PERIOD_MS));
  }
}

/* Only this task writes to UART, so messages cannot mix together. */
static void task_log(void *argument)
{
  (void)argument;
  const char *message;

  while (1)
  {
    xQueueReceive(log_queue, &message, portMAX_DELAY);
    USART_write_string(message);
  }
}

int main(void)
{
  init();

  log_queue = xQueueCreate(16, sizeof(const char *));

  /* Stack sizes are in words; LED has higher priority than UART logging. */
  xTaskCreate(task_led, "LED", 128, NULL, 2, NULL);
  xTaskCreate(task_data, "data", 128, NULL, 1, NULL);
  xTaskCreate(task_log, "log", 128, NULL, 1, NULL);

  /* FreeRTOS configures SysTick and starts running tasks. */
  vTaskStartScheduler();
}
