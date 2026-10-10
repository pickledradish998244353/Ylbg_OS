#include "keyboard.h"
#include "idt.h"
#include "io.h"
#include "printk.h"

// 第一套键盘扫描码 (Set 1 Scan Code) 到 ASCII 的映射表 (只列出了常用键)
static const char scancode_to_ascii[] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',    0, ' '
};

// 键盘中断回调函数 (IRQ1)
static void keyboard_callback(registers_t* regs) {
    (void)regs;

    // 从 0x60 端口读取键盘扫描码
    uint8_t scancode = rb(0x60);

    // 如果最高位是 1 (scancode & 0x80)，说明是松开按键 (Key Release)，我们暂时忽略
    if (scancode & 0x80) {
        return;
    }

    // 只处理在映射表范围内的按下事件
    if (scancode < sizeof(scancode_to_ascii)) {
        char ch = scancode_to_ascii[scancode];
        if (ch != 0) {
            printk("%c", ch);
        }
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback);
    uint8_t mask = rb(0x21);
    wb(0x21, mask & ~(1 << 1));
}