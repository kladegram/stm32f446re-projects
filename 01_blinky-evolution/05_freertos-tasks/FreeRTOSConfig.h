#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"

/* The CPU starts at 16 MHz; one scheduler tick is 1 ms. */
#define configCPU_CLOCK_HZ SystemCoreClock
#define configTICK_RATE_HZ 1000U
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configUSE_PREEMPTION 1
#define configMAX_PRIORITIES 4
#define configMINIMAL_STACK_SIZE 128U
#define configMAX_TASK_NAME_LEN 16

/* heap_4 supplies memory for task stacks and control blocks. */
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION 0
#define configTOTAL_HEAP_SIZE (8U * 1024U)
#define configUSE_MALLOC_FAILED_HOOK 1
#define configCHECK_FOR_STACK_OVERFLOW 2

#define configUSE_IDLE_HOOK 1
#define configUSE_TICK_HOOK 0
#define configUSE_TIMERS 0
#define configUSE_CO_ROUTINES 0
#define INCLUDE_vTaskDelay 1
/* Allow portMAX_DELAY to wait indefinitely for queue messages. */
#define INCLUDE_vTaskSuspend 1

/* STM32F446 uses four priority bits; larger numbers mean lower urgency. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15U
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
#define configKERNEL_INTERRUPT_PRIORITY \
  (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))

/* Connect the FreeRTOS port to the startup vector table. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Stop here on an error so its cause can be inspected in a debugger. */
#define configASSERT(condition) \
  do                           \
  {                            \
    if (!(condition))          \
    {                          \
      __disable_irq();         \
      for (;;)                 \
      {                        \
        __NOP();               \
      }                        \
    }                          \
  } while (0)

#endif
