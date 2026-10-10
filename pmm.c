#include "pmm.h"
#include "printk.h"

// 最大支持的物理内存
#define MAX_MEMORY_SIZE  (128 * 1024 * 1024) 
#define TOTAL_PAGES      (MAX_MEMORY_SIZE / PAGE_SIZE) // 32768 个页
#define BITMAP_SIZE      (TOTAL_PAGES / 8)             // 位图需要 4096 字节

static uint8_t pmm_bitmap[BITMAP_SIZE];
static uint32_t total_pages = TOTAL_PAGES;

extern uint32_t end;

static void bitmap_set(uint32_t page_index) {
    pmm_bitmap[page_index / 8] |= (1 << (page_index % 8));
}

static void bitmap_clear(uint32_t page_index) {
    pmm_bitmap[page_index / 8] &= ~(1 << (page_index % 8));
}

static int bitmap_test(uint32_t page_index) {
    return pmm_bitmap[page_index / 8] & (1 << (page_index % 8));
}

void pmm_init(multiboot_info_t* mb_info) {
    // 初始化内存为不可用
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    // 2. 检查 GRUB 是否成功传过来了 mmap 信息 (flags bit 6)
    if (!(mb_info->flags & (1 << 6))) {
        printk("[PMM Error] No multiboot memory map provided!\n");
        return;
    }

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mb_info->mmap_addr;
    uint32_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;

    while ((uint32_t)mmap < mmap_end) {
        // type == 1 表示这段内存是可用物理 RAM
        if (mmap->type == 1) {
            uint32_t start_addr = mmap->addr_low;
            uint32_t length = mmap->len_low;

            for (uint32_t addr = start_addr; addr < start_addr + length; addr += PAGE_SIZE) {
                uint32_t page_idx = addr / PAGE_SIZE;
                if (page_idx < total_pages) {
                    bitmap_clear(page_idx); // 标记为空闲
                }
            }
        }
        // 指向下一个内存区域块
        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
    // 保护前1MB内存 全部设置为1
    for (uint32_t addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        bitmap_set(addr / PAGE_SIZE);
    }

    // 4.2 保护内核自身占用的物理内存 (从 1MB 开始到内核 end 符号)
    uint32_t kernel_end_addr = (uint32_t)&end;
    for (uint32_t addr = 0x100000; addr < kernel_end_addr; addr += PAGE_SIZE) {
        bitmap_set(addr / PAGE_SIZE);
    }

    printk("[PMM] Physical Memory Manager initialized.\n");
    printk("[PMM] Kernel End Address: 0x%x\n", kernel_end_addr);
}

// 申请一个物理页
uint32_t pmm_alloc_page(void) {
    for (uint32_t page_idx = 0; page_idx < total_pages; page_idx++) {
        if (!bitmap_test(page_idx)) { // 找到一个 bit 为 0 的空闲页
            bitmap_set(page_idx);      // 标记为已分配
            uint32_t page_addr = page_idx * PAGE_SIZE;
            return page_addr;          // 返回物理起始地址
        }
    }
    printk("[PMM Error] Out of Memory!\n");
    return 0; // 内存耗尽
}

// 释放一个物理页
void pmm_free_page(uint32_t page_addr) {
    uint32_t page_idx = page_addr / PAGE_SIZE;
    if (page_idx < total_pages) {
        bitmap_clear(page_idx);
    }
}