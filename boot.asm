bits 32
section .multiboot
    align 4
    dd 0x1BADB002              ; Multiboot 魔数
    dd 0x00                    ; 标志位
    dd - (0x1BADB002 + 0x00)   ; 校验和

section .text
global _start
extern kernel_main

_start:
    cli                        ; 关中断
    mov esp, stack_top         ; 设置栈指针
    call kernel_main           ; 跳转到 C 语言入口
.hang:
    hlt                        ; 挂起 CPU
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384                 ; 分配 16KB 栈空间
stack_top: