bits 32
section .multiboot
align 8
multiboot_header:
    dd 0xE85250D6
    dd 0
    dd multiboot_header_end - multiboot_header
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))
    align 8
    dw 5
    dw 0
    dd 20
    dd 1920
    dd 1080
    dd 32
    align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:
section .bss
align 16
stack_bottom: resb 65536
stack_top:
section .text
global _start
extern kernel_main
_start:
    mov esp, stack_top
    call kernel_main
    cli
    hlt
    jmp $
