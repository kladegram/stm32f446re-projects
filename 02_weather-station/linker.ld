ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* The stack starts at the top of RAM and grows toward lower addresses. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Keep the hardware vector table at the first address in flash. */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH


    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab : { *(.ARM.extab*) } > FLASH
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* Variables run from RAM; their initial values are stored in flash. */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);


    /* Reserve RAM for zero-initialized variables; startup clears this region. */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Newlib allocates from end. Align to 8 bytes and check available RAM. */
    . = ALIGN(8);
    _end = .;
    PROVIDE(end = .);
    ASSERT(_end + 4096 <= _estack - 4096,
           "Reserve at least 4 KB each for the heap and stack")
}
