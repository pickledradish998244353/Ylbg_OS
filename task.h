#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#define TASK_RUNNING 1
#define TASK_DEAD    0

typedef struct task {
    uint32_t id;            // 任务 PID (offset 0)
    uint32_t esp;           // 挂起时的栈指针 ESP (offset 4) - 汇编中写死依赖此位置！
    uint32_t stack_bottom;  // 动态分配的栈基址
    uint32_t state;         // 任务状态 (RUNNING / DEAD)
    struct task* next;      // 链表指针，指向下一个任务
} task_t;

void task_init(void);
task_t* task_create(void (*entry)(void));
void schedule(void);
extern void switch_to(task_t* prev, task_t* next);

#endif