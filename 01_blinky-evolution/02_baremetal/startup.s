.syntax unified
.cpu cortex-m4
.thumb


.section .isr_vector, "a", %progbits

.word _estack
.word Reset_Handler


.section .text.Reset_Handler
.global Reset_Handler
.type Reset_Handler, %function

Reset_Handler:

    bl main

1:
    b 1b
