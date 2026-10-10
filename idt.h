#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

// IDT 描述符结构体
typedef struct {
    uint16_t base_low;    // ISR 入口地址低 16 位
    uint16_t selector;    // 代码段选择子（指向 GDT 中的内核代码段 0x08）
    uint8_t  always0;     // 保留，必须为 0
    uint8_t  flags;       // 类型和属性 (P|DPL|0|Type)
    uint16_t base_high;   // ISR 入口地址高 16 位
} __attribute__((packed)) idt_entry_t;

// IDT 指针（用于 lidt 指令）
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// 中断发生时的寄存器状态
typedef struct {
    uint32_t ds;                                     // 数据段
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha 压入
    uint32_t int_no, err_code;                       // 中断号和错误码
    uint32_t eip, cs, eflags, useresp, ss;           // CPU 自动压入
} registers_t;

// 中断处理函数指针类型
typedef void (*isr_t)(registers_t*);

void idt_init(void);
void register_interrupt_handler(uint8_t n, isr_t handler);

// IRQ 号（PIC 重映射后）
#define IRQ0  32   // 时钟
#define IRQ1  33   // 键盘
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#endif