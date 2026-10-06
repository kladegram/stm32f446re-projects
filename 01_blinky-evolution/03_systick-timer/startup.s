.syntax unified
.cpu cortex-m4
.thumb


.section .isr_vector, "a", %progbits

.word _estack
.word Reset_Handler
.word 0                  /* NMI */
.word 0                  /* HardFault */
.word 0                  /* MemManage */
.word 0                  /* BusFault */
.word 0                  /* UsageFault */
.word 0
.word 0
.word 0
.word 0
.word 0                  /* SVCall */
.word 0                  /* Debug monitor */
.word 0
.word 0                  /* PendSV */
.word SysTick_Handler    /* SysTick is Cortex-M exception 15 */


.section .text.Reset_Handler
.global Reset_Handler
.type Reset_Handler, %function

Reset_Handler:

    bl main

1:
    b 1b


.extern SysTick_Handler
