#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h" // 引入任务头文件

// 简单延时函数
void delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count * 100000; i++);
}

// 任务 A
void task_a(void) {
    while (1) {
        printk("A");
        delay(50);
    }
}

// 任务 B
void task_b(void) {
    while (1) {
        printk("B");
        delay(50);
    }
}

void kernel_main(uint32_t magic, multiboot_info_t* mb_info) {
    gdt_init();
    idt_init();

    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        pmm_init(mb_info);
    }

    vmm_init();
    heap_init();

    // 1. 初始化多任务系统 (将当前线程序号标记为 PID 0)
    task_init();

    // 2. 🚀 创建并发任务 A 和 B
    task_create(task_a);
    task_create(task_b);

    // 3. 启动定时器 (每 10ms 触发一次调度)
    timer_init(100);
    keyboard_init();

    printk("\n================ [Multitasking Started] ================\n");
    __asm__ volatile("sti");

    // 主任务死循环
    while (1) {
        __asm__ volatile("hlt");
    }
}