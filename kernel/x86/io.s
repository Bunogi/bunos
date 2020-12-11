.section .text
.global _x86_out_u8_string
_x86_out_u8_string:
    // Length
    movl 12(%esp), %ecx

    // do nothing on zero
    cmpl $0, %ecx
    je 2f

    // buffer
    movl %esi, -4(%esp) // ABI: preserve esi 
    movl 8(%esp), %esi
    // port
    movw 4(%esp), %dx

    cld // increment esi instead of decrementing it
1:
    outsb
    loop 1b

    movl -4(%esp), %esi
2:
    ret
