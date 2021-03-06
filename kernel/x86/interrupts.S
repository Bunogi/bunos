.section .data
idt_size:   
    .short 0
idt_offset: 
    .long 0

.section .text
.global load_idt_table
load_idt_table:
    movl 4(%esp), %eax // address
    movw 8(%esp), %cx // length

    movw %cx, idt_size
    movl %eax, idt_offset

    lidt idt_size

    // ABI: No need to preserve %cx or %eax
    ret

.macro int_handler v
.global _int_handler_vec\v
_int_handler_vec\v:
    pushl $\v
    jmp .isr_common_stub_error_code
.endm

.macro int_handler_noerror v
.global _int_handler_vec\v
_int_handler_vec\v:
    pushl $0
    pushl $\v
    jmp .isr_common_stub_no_code
.endm

#include <kernel/x86/handlers.inc>
#define error(_n) int_handler _n ;
#define noerror(_n) int_handler_noerror _n ;
HANDLERS(noerror, error)
#undef error
#undef noerror

//eax + ecx + edx + ebx + esp + ebp + esi + edi
.set pushad_length, (8 * 4)
    
.isr_common_stub_error_code:
    pushal

    // We have to be in a kernel segment for this to work, and
    // restore the previous segment later.
    movw %ds, %ax
    push %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    // Start a new stack frame here to not mess with the stack trace stuff
    movl $0, %ebp

    pushl %esp // Arg 1: exception data pointer

    cld // ABI needs DF to be clear 
    call _isr_callable_error_code

    // Clean up the stack
    addl $4, %esp

    pop %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    popal
    addl $8, %esp

    iret
    
.isr_common_stub_no_code:    
    pushal

    movw %ds, %ax
    push %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    // Start a new stack frame here to not mess with the stack trace stuff
    movl $0, %ebp

    pushl %esp // Arg 1: exception data pointer

    cld // ABI needs DF to be clear
    call _isr_callable_noerror

    addl $4, %esp

    pop %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    popal
    addl $8, %esp

    iret
