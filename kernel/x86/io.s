.section .text
.global _x86_out_u8_string
_x86_out_u8_string:
    // Length
    movl 12(%esp), %ecx

    // do nothing on zero
    cmpl $0, %ecx
    je 1f

    // buffer
    movl %esi, -4(%esp) // ABI: preserve esi
    movl 8(%esp), %esi
    // port
    movw 4(%esp), %dx

    cld // increment esi instead of decrementing it
    rep outsb
    //loop 1b

    movl -4(%esp), %esi
1:
    ret

.global _x86_in_u16_string
_x86_in_u16_string: 
    // Length
    movl 12(%esp), %ecx

    // do nothing on zero
    cmpl $0, %ecx
    je 1f

    // buffer
    movl %edi, -4(%esp) // ABI: preserve edi
    movl 8(%esp), %edi
    // port
    movw 4(%esp), %dx

    cld
    rep insw

    movl  -4(%esp), %edi
1:  
    ret

    
