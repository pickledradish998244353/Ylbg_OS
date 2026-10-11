#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

void kernel_main(uint32_t magic, multiboot_info_t* mb_info) {
    gdt_init();
    idt_init();
    printk("System initializing...\n");
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        pmm_init(mb_info);
    }

    vmm_init();
    heap_init();

    // 2. 🎯【测试 1】：动态申请小内存 (比如 32 字节结构体/字符串)
    char* str1 = (char*)kmalloc(32);
    char* str2 = (char*)kmalloc(64);

    printk("[Kmalloc Test] Allocated str1 (32B) at Virtual: 0x%x\n", str1);
    printk("[Kmalloc Test] Allocated str2 (64B) at Virtual: 0x%x\n", str2);

    // 往动态分配的内存写入内容
    str1[0] = 'O'; str1[1] = 'S'; str1[2] = '\0';
    printk("[Kmalloc Test] Value in str1: %s\n", str1);

    // 3. 🎯【测试 2】：释放与合并测试
    kfree(str1);
    printk("[Kmalloc Test] Freed str1\n");

    // 再次申请 16 字节，看是否能重用刚才释放的 str1 的内存块！
    char* str3 = (char*)kmalloc(16);
    printk("[Kmalloc Test] Allocated str3 (16B) at Virtual: 0x%x\n", str3);

    if (str3 == str1) {
        printk("[PASS] Kmalloc & Kfree heap memory reuse success!\n");
    }
    else {
        printk("[FAIL] Heap memory reuse failed.\n");
    }

    // 启动中断
    timer_init(100);
    keyboard_init();
    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
    }
}