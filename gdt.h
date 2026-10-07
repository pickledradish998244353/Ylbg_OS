#ifndef GDT_H
#define GDT_H
#include <stdint.h>

// 段描述符
typedef struct __attribute__((packed)) {
    uint16_t limit_low;     // 段界限 低 16 位
    uint16_t base_low;      // 段基址 低 16 位
    uint8_t  base_middle;   // 段基址 中 8 位
    uint8_t  access;        // 访问权限字节
    uint8_t  flag;         // 高 4 位: 标志位, 低 4 位: 段界限 高 4 位
    uint8_t  base_high;     // 段基址 高 8 位
} gdt_entry_t;

// gdtr 寄存器存储结构
typedef struct __attribute__((packed)) {
    uint16_t sz;          // GDT 表的大小
    uint32_t base;        // GDT 表的起始地址
} gdt_ptr_t;

void gdt_init(void);
// 加载 gdt 表并且刷新寄存器
extern void gdt_flush(uint32_t gdtr_ptr);

#endif