#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// 初始化 PIT 时钟中断，hz 为每秒中断的次数（例如 100Hz 表示每秒中断 100 次，即每 10ms 一次）
void timer_init(uint32_t hz);
uint32_t get_ticks(void);

#endif