.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

/* FreeRTOS needs SVC, PendSV and SysTick at their core exception positions. */
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
    .word SVC_Handler        /* Exception 11: start the first task */
    .word Default_Handler    /* Debug monitor */
    .word 0
    .word PendSV_Handler     /* Exception 14: switch tasks */
    .word SysTick_Handler    /* Exception 15: scheduler tick */

    /* External interrupt slots 0..96; no peripheral interrupts are used. */
    .rept 97
    .word Default_Handler
    .endr

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

    /* Clear zero-initialized variables, including the FreeRTOS state. */
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
