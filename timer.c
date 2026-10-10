#include "timer.h"
#include "idt.h"
#include "io.h"
#include "printk.h"

static uint32_t timer_ticks = 0;

// 1. 时钟中断的回调函数 (IRQ0 触发时会调到这里)
static void timer_callback(registers_t* regs) {
    timer_ticks++;
    // 为了防止屏幕刷屏太快，我们每 100 次 tick（大约每 1 秒）打印一次
    if (timer_ticks % 100 == 0) {
        printk("Tick: %d (1 second passed)\n", timer_ticks);
    }
}

// 2. 初始化 PIT 定时器
void timer_init(uint32_t hz) {
    // 步骤 2.1: 注册 IRQ0 (向量 32) 的处理函数
    register_interrupt_handler(32, timer_callback);

    // 步骤 2.2: 计算 PIT 的分频值
    // PIT 的硬件固有频率约为 1193182 Hz
    // 我们需要设置计数器计数到多少后产生一次中断
    uint32_t divisor = 1193180 / hz;

    // 步骤 2.3: 向控制端口 0x43 写入控制字
    // 0x36 = 000110110b:
    //   - 通道 0 (位 6-7: 00)
    //   - 先读写低字节，再读写高字节 (位 4-5: 11)
    //   - 模式 3：方波发生器 (位 1-3: 011)
    //   - 二进制计数 (位 0: 0)
    wb(0x43, 0x36);

    // 步骤 2.4: 拆分分频值并写入数据端口 0x40
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    wb(0x40, low);   // 先写低 8 位
    wb(0x40, high);  // 再写高 8 位

    // 步骤 2.5: 解除主 PIC 对 IRQ0 的屏蔽！
    // 0x21 是主 PIC 的 IMR (中断屏蔽寄存器)
    // 清零 bit 0 表示允许 IRQ0 (时钟中断) 通行
    uint8_t mask = rb(0x21);
    wb(0x21, mask & ~(1 << 0));
}

uint32_t get_ticks(void) {
    return timer_ticks;
}