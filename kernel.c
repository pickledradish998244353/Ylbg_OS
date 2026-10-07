#include "printk.h"
#include "vga.h"
#define MY_INT_MAX  2147483647
#define MY_INT_MIN  (-2147483647 - 1)

void kernel_main(void) {
    vga_init();

    while (1) {}
}