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
    cli
    mov esp, stack_top

    ;按照 C 语言调用约定 (cdecl)，从右往左压栈参数
    push ebx                    ; 第 2 个参数：multiboot_info 结构体指针 (里面包含内存地图 mmap_addr)
    push eax                    ; 第 1 个参数：multiboot magic 标志 (0x2BADB002)

    call kernel_main

.loop:
    hlt
    jmp .loop