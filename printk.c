#include "printk.h"
#include "vga.h"
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

// 给定整数转化为字符串
static void itoa_unsigned(uint32_t num, int base, char *buf) {
    char tmp[32];
    int i = 0;
    const char map[] = "0123456789abcdef";
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (num > 0) {
        tmp[i++] = map[num % base];
        num /= base;
    }
    int k = 0;
    for (int j = i - 1; j >= 0; j--) {
        buf[k++] = tmp[j];
    }
    buf[k] = '\0';
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            vga_putchar(fmt[i]);
            continue;
        }
        i++;
        if (fmt[i] == '\0') break;

        switch (fmt[i]) {
            // 1. 字符
            case 'c': {
                char c = (char)va_arg(args, int); // char 在可变参数中提升为 int
                vga_putchar(c);
                break;
            }
            // 字符串
            case 's': {
                const char *s = va_arg(args, const char*);
                if (!s) s = "(null)";
                vga_puts(s);
                break;
            }
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                uint32_t uval;
                if (val < 0) {
                    vga_putchar('-');
                    uval = -(uint32_t)val;
                } else {
                    uval = (uint32_t)val;
                }
                char buf[32];
                itoa_unsigned(uval, 10, buf);
                vga_puts(buf);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(args, uint32_t);
                char buf[32];
                itoa_unsigned(val, 10, buf);
                vga_puts(buf);
                break;
            }
            // 十六进制
            case 'x': {
                uint32_t val = va_arg(args, uint32_t);
                char buf[32];
                itoa_unsigned(val, 16, buf);
                vga_puts(buf);
                break;
            }
            case 'p': {
                uint32_t val = (uint32_t)va_arg(args, void*);
                vga_puts("0x");
                char buf[32];
                itoa_unsigned(val, 16, buf);
                vga_puts(buf);
                break;
            }
            case '%': {
                vga_putchar('%');
                break;
            }
            default: {
                vga_putchar('%');
                vga_putchar(fmt[i]);
                break;
            }
        }
    }
    va_end(args);
}