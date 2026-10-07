#include "vga.h"
#include "printk.h"
#include "gdt.h"

void kernel_main(void) {
    vga_init();

    printk("=== Kernel Boot ===\n");
    gdt_init();
    printk("=== GDT Ready, Kernel Running! ===\n");

    while (1) {}
}