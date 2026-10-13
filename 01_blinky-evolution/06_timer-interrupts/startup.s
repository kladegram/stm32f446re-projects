.syntax unified
.cpu cortex-m4
.thumb
/* Core exceptions occupy entries 0..15, followed by external interrupts. */
.section .isr_vector, "a", %progbits
.balign 512
.global g_pfnVectors
g_pfnVectors:
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

/* External IRQs 0..27 precede TIM2, which is IRQ 28 (vector entry 44). */
.rept 28
.word Default_Handler
.endr
.word TIM2_IRQHandler

/* External IRQs 29..39 precede the shared EXTI10..15 interrupt. */
.rept 11
.word Default_Handler
.endr
.word EXTI15_10_IRQHandler    /* IRQ 40 (vector entry 56): user button on PC13 */

/* Remaining external IRQs 41..96 are unused in this project. */
.rept 56
.word Default_Handler
.endr
.size g_pfnVectors, .-g_pfnVectors

.section .text.Reset_Handler, "ax", %progbits
.global Reset_Handler
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    /* Copy initialized variables from flash to RAM. */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b

    /* Clear zero-initialized variables. */
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
    bl SystemInit
    bl main
    b Default_Handler
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler, "ax", %progbits
.global Default_Handler
.type Default_Handler, %function
.thumb_func
Default_Handler:
    b Default_Handler
.size Default_Handler, .-Default_Handler

.section .note.GNU-stack, "", %progbits
