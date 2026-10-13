// task.h
#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// 任务状态定义
typedef enum task_state {
    TASK_RUNNING = 0, // 正在运行或准备运行
    TASK_SLEEPING,    // 睡眠中（等待时钟中断唤醒）
    TASK_DEAD         // 已死亡（等待主调度器回收资源）
} task_state_t;

typedef struct task {
    uint32_t id;             // 任务 PID
    uint32_t esp;            // 任务当前的栈指针 (上下文切换用)
    uint32_t* stack;         // 任务分配的初始栈基地址 (用于 kfree 释放)
    task_state_t state;      // 任务状态
    uint32_t sleep_ticks;    // 剩余睡眠的时钟滴答数
    struct task* next;       // 循环链表下一个结点
} task_t;

void task_init(void);
task_t* task_create(void (*entry)(void));
void task_exit(void);
void task_sleep(uint32_t ticks); // 睡眠指定的 ticks 数量
void schedule(void);              // 核心调度函数
void task_show(void);             // 打印当前系统中的任务列表 (类似 ps)

#endif