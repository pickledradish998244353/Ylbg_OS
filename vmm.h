#ifndef VMM_H
#define VMM_H

#include <stdint.h>

// 页表项 / 页目录项 标志位 (Bit 0 ~ 2)
#define VMM_PAGE_PRESENT   0x1  // P 位: 1 表示页在内存中
#define VMM_PAGE_WRITABLE  0x2  // R/W 位: 1 表示可读写，0 表示只读
#define VMM_PAGE_USER      0x4  // U/S 位: 1 表示用户态可访问，0 表示仅内核态可访问

// 初始化虚拟内存管理并开启硬件分页
void vmm_init(void);

// 将一个虚拟地址 (va) 映射到 物理地址 (pa)
void map_page(uint32_t va, uint32_t pa, uint32_t flags);

#endif