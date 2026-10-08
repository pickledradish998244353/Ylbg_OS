[BITS 32]

; ==================== 加载 IDT ====================
global idt_flush
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        cli
        push 0              ; 占位 error code
        push %1             ; 中断号
        jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli
        push %1             ; 中断号（error code 已被 CPU 压入）
        jmp isr_common_stub
%endmacro

; 定义 0-31 号 CPU 异常
ISR_NOERRCODE 0    ; Divide By Zero
ISR_NOERRCODE 1    ; Debug
ISR_NOERRCODE 2    ; NMI
ISR_NOERRCODE 3    ; Breakpoint
ISR_NOERRCODE 4    ; Overflow
ISR_NOERRCODE 5    ; Bound Range Exceeded
ISR_NOERRCODE 6    ; Invalid Opcode
ISR_NOERRCODE 7    ; Device Not Available
ISR_ERRCODE   8    ; Double Fault
ISR_NOERRCODE 9    ; Coprocessor Segment Overrun
ISR_ERRCODE   10   ; Invalid TSS
ISR_ERRCODE   11   ; Segment Not Present
ISR_ERRCODE   12   ; Stack-Segment Fault
ISR_ERRCODE   13   ; General Protection Fault
ISR_ERRCODE   14   ; Page Fault
ISR_NOERRCODE 15   ; Reserved
ISR_NOERRCODE 16   ; x87 FPU
ISR_ERRCODE   17   ; Alignment Check
ISR_NOERRCODE 18   ; Machine Check
ISR_NOERRCODE 19   ; SIMD FP
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; ==================== IRQ 宏（硬件中断，重映射到 32-47） ====================
%macro IRQ 2
    global irq%1
    irq%1:
        cli
        push 0
        push %2
        jmp irq_common_stub
%endmacro

IRQ 0,  32   ; 时钟
IRQ 1,  33   ; 键盘
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; ==================== ISR 公共入口 ====================
extern isr_handler
isr_common_stub:
    pusha                   ; 保存 8 个通用寄存器
    mov ax, ds
    push eax                ; 保存 ds

    mov ax, 0x10            ; 加载内核数据段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                ; 传参 registers_t*
    call isr_handler
    add esp, 4

    pop eax                 ; 恢复 ds
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8              ; 跳过 int_no 和 err_code
    sti
    iret

; ==================== IRQ 公共入口 ====================
extern irq_handler
irq_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iret