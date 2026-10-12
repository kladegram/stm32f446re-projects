/* Idle and error callbacks enabled in FreeRTOSConfig.h. */
#include "FreeRTOS.h"
#include "task.h"

/* Wait for an interrupt when no application task is ready. */
void vApplicationIdleHook(void)
{
  __WFI();
}

void vApplicationMallocFailedHook(void)
{
  configASSERT(0);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
  (void)task;
  (void)task_name;
  configASSERT(0);
}
