#include "task.h"
#include "heap.h"
#include "printk.h"

static task_t* current_task = nullptr;
static task_t* task_chain = nullptr; // 任务循环链表
static uint32_t next_pid = 0;

// 当任务函数执行完毕 return 时，会跳到这里进行收尾
static void task_exit(void) {
    printk("[Task] Task %d exited.\n", current_task->id);
    current_task->state = TASK_DEAD;
    while (1) {
        __asm__ volatile("hlt"); // 死亡任务挂起
    }
}

void task_init(void) {
    // 1. 为当前的 kernel_main 创建主任务 (PID 0)
    current_task = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->next = current_task; // 自环链表

    task_chain = current_task;
    printk("[Task] Multitasking initialized. Main Task ID: 0\n");
}

task_t* task_create(void (*entry)(void)) {
    task_t* t = (task_t*)kmalloc(sizeof(task_t));
    uint32_t stack = (uint32_t)kmalloc(4096);

    t->id = next_pid++;
    t->stack_bottom = stack;
    t->state = TASK_RUNNING;

    // 伪造栈帧！在 4KB 栈顶手动摆放 switch_to 期待的数据：
    uint32_t* top = (uint32_t*)(stack + 4096);

    *(--top) = (uint32_t)task_exit; // 顶层 1：退向地址 (任务函数 return 后跳到 task_exit)
    *(--top) = (uint32_t)entry;     // 顶层 2：EIP 入口 (switch_to 的 ret 指令会跳到这里！)
    *(--top) = 0x202;               // EFLAGS (0x202 表示开启 IF 中断标志位)
    
    // pusha 对应的 8 个通用寄存器 (初始全部清零)
    *(--top) = 0; // EAX
    *(--top) = 0; // ECX
    *(--top) = 0; // EDX
    *(--top) = 0; // EBX
    *(--top) = 0; // ESP (dummy)
    *(--top) = 0; // EBP
    *(--top) = 0; // ESI
    *(--top) = 0; // EDI

    t->esp = (uint32_t)top;

    // 将新任务插入循环链表
    t->next = task_chain->next;
    task_chain->next = t;

    printk("[Task] Created Task %d at entry 0x%x\n", t->id, entry);
    return t;
}

// task 调度器
void schedule(void) {
    if (!current_task) return;

    task_t* next = current_task->next;

    // 跳过已死亡的任务
    while (next->state != TASK_RUNNING) {
        next = next->next;
        if (next == current_task) return;
    }

    if (next == current_task) return;

    task_t* prev = current_task;
    current_task = next;

    // 执行核心上下文切换！
    switch_to(prev, current_task);
}