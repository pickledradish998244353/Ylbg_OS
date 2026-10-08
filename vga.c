#include "vga.h"
#include <stddef.h>
#include "io.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

// 光标位置
static size_t lx = 0;
static size_t ly = 0;
static uint8_t cur_col = 0x0F;

// 将前景色和背景色打包成一个字节的属性
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

// 将字符和属性打包成一个 16 位的 VGA 显存单元
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

// 更新显卡硬件光标的位置
static void vga_update_hardware_cursor(void) {
    uint16_t pos = ly * VGA_WIDTH + lx;
    wb(0x3D4, 0x0F); // 设置低 8 位
    wb(0x3D5, (uint8_t)(pos & 0xFF));
    wb(0x3D4, 0x0E); // 设置高 8 位
    wb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// 初始化 VGA 驱动
void vga_init(void) {
    lx = 0;
    ly = 0;
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear();
}


void vga_set_color(vga_color_t fg, vga_color_t bg) {
    cur_col = vga_entry_color(fg, bg);
}

// 清屏
void vga_clear(void) {
    uint16_t blank = vga_entry(' ', cur_col);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = blank;
    }
    lx = 0;
    ly = 0;
    vga_update_hardware_cursor();
}

// 屏幕滚屏逻辑
static void vga_scroll(void) {
    if (ly >= VGA_HEIGHT) {
        for (size_t j = 1; j  < VGA_HEIGHT; ++j) {
            for (size_t i = 0; i < VGA_WIDTH; ++i) {
                size_t cur = j * VGA_WIDTH + i;
                size_t lst = (j - 1) * VGA_WIDTH + i;
                VGA_MEMORY[lst] = VGA_MEMORY[cur];
            }
        }

        uint16_t blk = vga_entry(' ', cur_col);
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blk;
        }
        ly = VGA_HEIGHT - 1;
        lx = 0;
    }
}

// 输出单个字符
void vga_putchar(char c) {
    // 换行
    if (c == '\n') {
        lx = 0;
        ly++;
    }
    // 回车
    else if (c == '\r') {
       lx = 0;
    }
    // 退格
    else if (c == '\b') {
        if (lx > 0) {
            lx--;
            VGA_MEMORY[ly * VGA_WIDTH + lx] = vga_entry(' ', cur_col);
        }
    }
    // tab
    else if (c == '\t') {
        lx = (lx + 4) & (~3);
    }
    // 输出字符
    else {
        const size_t idx = ly * VGA_WIDTH + lx;
        VGA_MEMORY[idx] = vga_entry(c, cur_col);
        lx++;
    }
    if (lx >= VGA_WIDTH) {
        lx = 0;
        ly++;
    }

    // 如果超过VGA大小滚屏
    vga_scroll();
    // 移动光标
    vga_update_hardware_cursor();
}

// 输出字符串
void vga_puts(const char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        vga_putchar(s[i]);
    }
}