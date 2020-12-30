ENTRY(_start)

SECTIONS
{
    . = 0x00100000;
    /* TODO: use a file directive instead to ensure that the boot.s.o file is linked first somehow */
    /* Otherwise the offset of the header in the elf becomes wrong */
    .multiboot.data : {
       LONG(0x1BADB002);
       LONG(3);
       LONG(-(3 + 0x1BADB002));
       /* *(.multiboot.data) */
    }
    
    .multiboot.text : {
        *(.multiboot.text)
    }

    . += 0xC0000000;

    /* Store the section from 0xC0101000. */
    . = ALIGN(4K);
    _kernel_start = .;
    _kernel_nonwritable_start = .;
    .text ALIGN(4K): AT (ADDR (.text) - 0xC0000000)
    {
        *(.text)
    }
    .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata)
    }
    . = ALIGN(4K);
    _kernel_nonwritable_end = .;
 
    _kernel_writable_start = .;
    .data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
    }
 
    .bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }
    _kernel_writable_end = .;
    _kernel_end = .;

   /* Reserve some(static) heap space */
   . = ALIGN(4K);
   _kernel_heap_start = .;
   . += 0x00100000;
   _kernel_heap_end = .;
}
