.section .data
gdtr:
    .short 0
    .long 0

.section .text
.global load_gdt
.type load_gdt, @function
load_gdt:
    movl 4(%esp), %eax
    movl %eax, (gdtr + 2)
    movw 8(%esp), %ax
    movw %ax, gdtr

    lgdt gdtr

    jmp $8, $.reload_segments
//jmp $.reload_segments, 8
.reload_segments:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret
    
