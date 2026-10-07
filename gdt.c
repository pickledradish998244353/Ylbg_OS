#include "gdt.h"
#include "printk.h"

#define GDT_ENTRIES 3

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdtr;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran) {
    gdt[index].base_low    = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high   = (base >> 24) & 0xFF;
    gdt[index].limit_low   = (limit & 0xFFFF);
    gdt[index].flag = (limit >> 16) & 0x0F;
    gdt[index].flag |= (gran & 0xF0);
    gdt[index].access = access;
}

// 初始化gdt 表
void gdt_init(void) {
    gdtr.sz = sizeof(gdt) - 1;
    gdtr.base  = (uint32_t)&gdt;
    // null 段
    gdt_set_entry(0, 0, 0, 0, 0);
    // 内核代码段
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);
    // 内核数据段
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);
    // 将gdt 加载到内存 并且刷新寄存器
    gdt_flush((uint32_t)&gdtr);
    printk("[GDT] GDT loaded successfully!\n");
    printk("[GDT] GDTR base=0x%x, limit=%d\n", gdtr.base, gdtr.sz);
}