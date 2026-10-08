#include "idt.h"
#include "io.h"
#include "printk.h"

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idtp;

// 中断处理函数表
static isr_t interrupt_handlers[IDT_ENTRIES];

// 汇编中声明的函数
extern void idt_flush(uint32_t);

// 32 个 CPU 异常 stub
extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

// 16 个 IRQ stub
extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// 设置 IDT 的一项
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;  // 0x8E = P=1, DPL=00, Type=1110 (32位中断门)
}

// ==================== 8259 PIC 重映射 ====================
// 默认 IRQ0-7 → 0x08-0x0F（和 CPU 异常冲突！）
// 重映射到 0x20-0x2F
static void pic_remap(void) {
    // ICW1: 初始化
    wb(0x20, 0x11);
    wb(0xA0, 0x11);

    // ICW2: 设置中断向量偏移
    wb(0x21, 0x20);   // 主片：IRQ0 → 0x20 (32)
    wb(0xA1, 0x28);   // 从片：IRQ8 → 0x28 (40)

    // ICW3: 配置主从关系
    wb(0x21, 0x04);   // 主片：IRQ2 连接从片
    wb(0xA1, 0x02);   // 从片：连接到主片 IRQ2

    // ICW4: 8086 模式
    wb(0x21, 0x01);
    wb(0xA1, 0x01);

    // 屏蔽所有中断（后面按需开启）
    wb(0x21, 0xFF);
    wb(0xA1, 0xFF);
}

// ==================== IDT 初始化 ====================
void idt_init(void) {
    idtp.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idtp.base  = (uint32_t)&idt;

    // 清零
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }

    // 重映射 PIC
    pic_remap();

    // 注册 CPU 异常（0x08 = 内核代码段选择子，0x8E = 32 位中断门）
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    // 注册 IRQ（映射后：32-47）
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    // 加载 IDT
    idt_flush((uint32_t)&idtp);
}

// 注册中断处理函数
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// ==================== 异常名称表（便于调试） ====================
static const char* exception_messages[] = {
    "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
    "Into Detected Overflow", "Out of Bounds", "Invalid Opcode", "No Coprocessor",
    "Double Fault", "Coprocessor Segment Overrun", "Bad TSS", "Segment Not Present",
    "Stack Fault", "General Protection Fault", "Page Fault", "Unknown Interrupt",
    "Coprocessor Fault", "Alignment Check", "Machine Check", "SIMD FP Exception",
    "Reserved","Reserved","Reserved","Reserved",
    "Reserved","Reserved","Reserved","Reserved",
    "Reserved","Reserved","Reserved","Reserved"
};

// ==================== C 层 ISR 总入口（异常） ====================
void isr_handler(registers_t* regs) {
    // 如果注册了自定义的处理函数（比如 3 号断点处理）
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
        return;
    }

    // 默认未捕获的 CPU 异常打印并停机
    if (regs->int_no < 32) {
        printk("[Exception] %s (int=%d, err=%d)\n",
               exception_messages[regs->int_no], regs->int_no, regs->err_code);
        for (;;) __asm__ volatile("cli; hlt");
    }
}

void irq_handler(registers_t* regs) {
    // 向 PIC 发送 EOI（End of Interrupt）
    if (regs->int_no >= 40) {
        wb(0xA0, 0x20);   // 从片
    }
    wb(0x20, 0x20);       // 主片

    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}