/* Linker script for Tosin RTOS kernel */

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 32KB (0x8000) */
    . = 0x8000;
    
    .text : ALIGN(4096)
    {
        *(.text)
        *(.text.*)
    }
    
    .rodata : ALIGN(4096)
    {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : ALIGN(4096)
    {
        *(.data)
        *(.data.*)
    }
    
    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* End of kernel */
    _kernel_end = .;
}
