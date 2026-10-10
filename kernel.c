#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"

void kernel_main(uint32_t magic, multiboot_info_t* mb_info) {
    gdt_init();
    idt_init();

    printk("System initializing...\n");

    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        pmm_init(mb_info);
    }

    // 1. 初始化虚拟内存管理并开启分页！
    vmm_init();

    // 2. 🎯【神奇测试】：虚拟地址映射测试！
    // 申请一个真实的物理页
    uint32_t phys_page = pmm_alloc_page();

    // 选一个完全不相关的虚拟地址，比如 0x60000000 (1.5GB 处)
    uint32_t virt_addr = 0x60000000;

    // 用 map_page 把虚拟地址 0x60000000 强行映射到刚才分配的物理页上！
    map_page(virt_addr, phys_page, VMM_PAGE_WRITABLE);

    printk("[VMM Test] Mapped Virtual 0x%x ---> Physical 0x%x\n", virt_addr, phys_page);

    // 尝试往虚拟地址 0x60000000 写入数值
    uint32_t* ptr = (uint32_t*)virt_addr;
    *ptr = 0x88888888;

    // 从真实的物理地址读取出来验证！
    uint32_t* phys_ptr = (uint32_t*)phys_page;

    if (*phys_ptr == 0x88888888) {
        printk("[PASS] Paging Magic Success! Value at Phys 0x%x is 0x%x\n", phys_page, *phys_ptr);
    } else {
        printk("[FAIL] Paging Mapping Failed!\n");
    }

    // 启动定时器与键盘
    timer_init(100);
    keyboard_init();
    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
    }
}