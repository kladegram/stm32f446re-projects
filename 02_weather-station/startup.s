.syntax unified
.cpu cortex-m4
.thumb
/* Cortex-M4 exception vectors: initial stack pointer, then handler addresses. */
.section .isr_vector, "a", %progbits

.word _estack
.word Reset_Handler
.word Default_Handler    /* NMI */
.word Default_Handler    /* HardFault */
.word Default_Handler    /* MemManage */
.word Default_Handler    /* BusFault */
.word Default_Handler    /* UsageFault */
.word 0
.word 0
.word 0
.word 0
.word Default_Handler    /* SVCall */
.word Default_Handler    /* Debug monitor */
.word 0
.word Default_Handler    /* PendSV */
.word Default_Handler    /* SysTick */

/* External Interrupts */
/*
.rept 97
.word Default_Handler
.endr
*/
/*
.rept 28
.word Default_Handler
.endr
.word     TIM2_IRQHandler                 
.rept 68
.word Default_Handler
.endr
*/
.rept 38
.word Default_Handler
.endr
.word     USART2_IRQHandler
.word Default_Handler
.word     EXTI15_10_IRQHandler                 
.rept 56
.word Default_Handler
.endr

.section .text.Reset_Handler, "ax", %progbits
.global Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:
    /* .data: load initial values from flash into their working addresses in RAM. */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b

    /* .bss: C requires variables without explicit values to start at zero. */
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1], #4
    b 3b

4:
    /* All RAM initialization is complete; call the C application. */
    bl main
    b Default_Handler
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler, "ax", %progbits
.global Default_Handler
.type Default_Handler, %function
.thumb_func
Default_Handler:
    /* Stop here if an unexpected exception occurs or main returns. */
    b Default_Handler
.size Default_Handler, .-Default_Handler

.section .note.GNU-stack, "", %progbits
