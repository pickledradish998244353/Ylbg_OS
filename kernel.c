#include "printk.h"
#include "vga.h"
#include "gdt.h"
#include "idt.h"

void kernel_main(void) {
    // 1. 优先初始化 VGA 显存（清屏 + 设置白字黑底 + 重置光标）
    vga_init();

    printk("Hello MyOS! Kernel booting...\n\n");

    // 2. 初始化 GDT
    gdt_init();
    printk("[OK] GDT Initialized.\n");

    // 3. 初始化 IDT
    idt_init();
    printk("[OK] IDT Initialized.\n");

    // 4. 测试主动触发 3 号异常 (Breakpoint)
    printk("\nTesting Interrupt: Triggering int $0x03...\n");
    __asm__ volatile("int $0x03");

    printk("\n[SUCCESS] Returned from Interrupt 0x03 safely!\n");

    for (;;) {
        __asm__ volatile("hlt");
    }
}