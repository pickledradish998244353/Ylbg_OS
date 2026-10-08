[BITS 32]

; ==================== 1. Multiboot 引导头 ====================
MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; ==================== 2. 分配 16KB 内核栈空间 (关键新增！) ====================
section .bss
align 16
stack_bottom:
    resb 16384                 ; 预留 16 KB 的栈内存
stack_top:

; ==================== 3. 内核入口 ====================
section .text
global start
extern kernel_main

start:
    cli                         ; 关中断
    mov esp, stack_top          ; 【最关键的一行】：把栈指针 ESP 设置为栈顶！

    call kernel_main            ; 现在 C 语言函数可以安全地使用栈了！

.loop:
    hlt
    jmp .loop