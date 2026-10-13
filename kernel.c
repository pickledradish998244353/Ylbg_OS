// kernel.c
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h"

void task_a(void) {
    while (1) {
        printk("A");
        task_sleep(10); // 睡 10 个 tick
    }
}

void task_b(void) {
    for (int i = 0; i < 5; i++) {
        printk("B");
        task_sleep(15); // 睡 15 个 tick
    }
    printk("\n[Task B Finished]\n");
}

void kernel_main(uint32_t magic, multiboot_info_t* mb_info) {
    gdt_init();
    idt_init();

    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        pmm_init(mb_info);
    }

    vmm_init();
    heap_init();

    task_init(); // 初始化 (创建 PID 0 和 PID 1 Idle 任务)

    task_create(task_a); // PID 2
    task_create(task_b); // PID 3

    timer_init(100);
    keyboard_init();

    printk("\n================ [Multitasking Started] ================\n");
    __asm__ volatile("sti");

    // 主任务死循环输出 "."，验证多任务正常交替
    while (1) {
        printk(".");
        task_sleep(50);
    }
}