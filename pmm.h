#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "multiboot.h"

#define PAGE_SIZE 4096

// 初始化物理内存管理器
void pmm_init(multiboot_info_t* mb_info);

// 分配一个 4KB 物理页，返回物理起始地址 (若失败返回 0)
uint32_t pmm_alloc_page(void);

// 释放一个 4KB 物理页
void pmm_free_page(uint32_t page_addr);

#endif