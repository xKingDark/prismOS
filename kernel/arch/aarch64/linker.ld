ENTRY(_start)

__stack_size = 128K;      /* Boot stack size */

SECTIONS {
    . = 0x40080000;

    /* Text section */
    .text : ALIGN(4K) {
        __text_start = .;
        *(.text .text.*)
        __text_end = .;
    }

    /* Read-only data */
    .rodata : ALIGN(4K) {
        __rodata_start = .;
        *(.rodata*)
        __rodata_end = .;
    }

    /* Global constructors (init_array) */
    .init_array : ALIGN(8) {
        PROVIDE(__init_array_start = .);
        KEEP(*(.init_array .init_array.*))
        PROVIDE(__init_array_end = .);
    }

    /* Global destructors (fini_array) */
    .fini_array : ALIGN(8) {
        PROVIDE(__fini_array_start = .);
        KEEP(*(.fini_array .fini_array.*))
        PROVIDE(__fini_array_end = .);
    }

    /* Data section */
    .data : ALIGN(4K) {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    }

    /* BSS section */
    .bss (NOLOAD) : ALIGN(4K) {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* Single boot CPU stack */
    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . = . + __stack_size;
        . = ALIGN(16);
        _stack_top = .;
    }

    /* End of the kernel image in memory */
    . = ALIGN(4K);
    _image_end = .;
    PROVIDE(end = .);
    PROVIDE(_end = .);

    /* Heap section */
    .heap (NOLOAD) : {
        _heap_start = .;
        . = . + 0x4000000; /* Reserve 64 MB for heap */
        _heap_end = .;
    }
}