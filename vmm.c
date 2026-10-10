#include "vmm.h"
#include "pmm.h"
#include "printk.h"

// 页目录指针 (必须是 4KB 对齐的物理地址)
static uint32_t* page_directory = 0;

// C 语言内联汇编：把页目录物理地址装入 CR3 寄存器
static inline void load_page_directory(uint32_t* pd_addr) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pd_addr));
}

// C 语言内联汇编：开启 CR0 寄存器的 PG (Paging) 位 (bit 31)
static inline void enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // 将最高位置 1，正式开启硬件分页
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

// 建立 虚拟地址 (va) -> 物理地址 (pa) 的映射
void map_page(uint32_t va, uint32_t pa, uint32_t flags) {
    // 1. 拆分 32 位虚拟地址！(利用你刚才总结的 10 + 10 + 12 规则)
    uint32_t pd_index = va >> 22;               // 高 10 位：页目录索引 (0~1023)
    uint32_t pt_index = (va >> 12) & 0x3FF;     // 中间 10 位：页表索引 (0~1023)

    uint32_t* page_table = 0;

    // 2. 检查一级索引 (页目录项 PDE) 是否存在
    if (page_directory[pd_index] & VMM_PAGE_PRESENT) {
        // 如果存在，提取出二级页表的物理首地址 (抹去低 12 位标志)
        page_table = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    } else {
        // 如果不存在，用 pmm 动态申请一个 4KB 物理页用来当“二级页表”！
        page_table = (uint32_t*)pmm_alloc_page();

        // 将新建的页表清零
        for (int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }

        // 把这个新建页表的物理地址填进一级页目录中，并设为 Present + Writable
        page_directory[pd_index] = (uint32_t)page_table | VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE | flags;
    }

    // 3. 将物理页地址填入二级页表 (PTE)
    // 真正的物理页首地址 (pa & 0xFFFFF000) + 属性标志位
    page_table[pt_index] = (pa & 0xFFFFF000) | VMM_PAGE_PRESENT | flags;
}

void vmm_init(void) {
    // 1. 用 PMM 申请 1 个 4KB 物理页，作为全局唯一的“页目录” (Page Directory)
    page_directory = (uint32_t*)pmm_alloc_page();

    // 清空页目录 (把 1024 个 PDE 全部设为 0，表示 Present 位为 0)
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
    }

    // 把前 8MB 的物理内存 (0x00000000 ~ 0x00800000) 映射到完全相同的虚拟地址上！
    // 这保证了内核代码、VGA显存、GDT/IDT 在开启分页后依然能安全运行，绝不出错！
    for (uint32_t addr = 0; addr < 0x800000; addr += PAGE_SIZE) {
        map_page(addr, addr, VMM_PAGE_WRITABLE);
    }

    // 3. 将页目录地址写入 CR3
    load_page_directory(page_directory);

    enable_paging();

    printk("[VMM] Virtual Memory Manager initialized & Paging Enabled!\n");
}