/* Linker script for STM32F767ZI */

_Stack_Size = 128K;  /* use all of DTCM for stack */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM2 (rwx) : ORIGIN = 0x2007C000, LENGTH = 16K
    SRAM1 (rwx) : ORIGIN = 0x20020000, LENGTH = 368K
    DTCM  (rw)  : ORIGIN = 0x20000000, LENGTH = 128K
    ITCM  (rx)  : ORIGIN = 0x00000000, LENGTH = 16K
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        _stext = .;
        *(.text*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .init :
    {
        . = ALIGN(4);
        KEEP (*(.init))
        . = ALIGN(4);
    } >FLASH

    .fini :
    {
        . = ALIGN(4);
        KEEP (*(.fini))
        . = ALIGN(4);
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    .preinit_array    :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH

    .init_array    :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH

    .fini_array    :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* initialized data */
    _sidata = LOADADDR(.data);
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM1 AT>FLASH

    /* uninitialized data */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } >SRAM1
    PROVIDE(end = .);

    /* check that there is enough remaining space for the stack */
    _estack = ORIGIN(DTCM) + LENGTH(DTCM) - 1;
    _sstack = _estack - _Stack_Size;
    .user_stack :
    {
        . = ALIGN(4);
        . = . + _Stack_Size;
        . = ALIGN(4);
    } >DTCM

    /* discard sections */
    /DISCARD/ :
    {
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
