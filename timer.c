#include "timer.h"
#include "idt.h"
#include "io.h"
#include "task.h"

static uint32_t timer_ticks = 0;

// 1. 时钟中断的回调函数 (IRQ0 触发时会调到这里)
static void timer_callback(registers_t* regs) {
    (void)regs;
    timer_ticks++;
    // 时钟中断产生 触发调度器
    schedule();
}
// 2. 初始化 PIT 定时器
void timer_init(uint32_t hz) {
    register_interrupt_handler(32, timer_callback);
    uint32_t divisor = 1193180 / hz;
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