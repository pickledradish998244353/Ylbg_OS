#include "task.h"
#include "heap.h"
#include "printk.h"

static task_t* current_task = 0;
static task_t* task_chain = 0;
static uint32_t next_pid = 0;

extern void switch_to(task_t* prev, task_t* next);

// 系统空闲任务 (当所有任务都在 sleep 时执行)
static void idle_task(void) {
    while (1) {
        __asm__ volatile("hlt"); // 保底 hlt，等待时钟中断唤醒其他任务
    }
}

void task_init(void) {
    // 1. 为 kernel_main 创建主任务 (PID 0)
    current_task = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->sleep_ticks = 0;
    current_task->stack = 0;
    current_task->next = current_task;

    task_chain = current_task;

    // 2. 创建系统 Idle 任务 (PID 1) 作为保底
    task_create(idle_task);

    printk("[Task] Multitasking initialized. Idle Task Created.\n");
}

task_t* task_create(void (*entry)(void)) {
    task_t* t = (task_t*)kmalloc(sizeof(task_t));

    // 分配 4KB 的内核栈空间
    uint32_t* stack = (uint32_t*)kmalloc(4096);
    // esp 指向栈顶 (高地址)
    uint32_t* esp = (uint32_t*)((uint32_t)stack + 4096);

    // -----------------------------------------------------------------
    // 严格匹配 task.asm 中的栈结构 (从高地址到低地址压栈):
    // -----------------------------------------------------------------

    // 1. 任务退出的返回地址 (当 entry 执行完 return 时跳转)
    *(--esp) = (uint32_t)task_exit;

    // 2. 任务入口地址 (对应 asm 中的 ret 指令要跳转的目标 EIP)
    *(--esp) = (uint32_t)entry;
    *(--esp) = 0x202;

    *(--esp) = 0; // EDI
    *(--esp) = 0; // ESI
    *(--esp) = 0; // EBP
    *(--esp) = 0; // ESP
    *(--esp) = 0; // EBX
    *(--esp) = 0; // EDX
    *(--esp) = 0; // ECX
    *(--esp) = 0; // EAX

    // -----------------------------------------------------------------

    t->id = next_pid++;
    t->esp = (uint32_t)esp; // 保存当前的栈顶指针，供 switch_to 恢复现场
    t->stack = stack;
    t->state = TASK_RUNNING;
    t->sleep_ticks = 0;

    // 插入到循环链表中
    t->next = task_chain->next;
    task_chain->next = t;

    printk("[Task] Created Task ID: %d\n", t->id);
    return t;
}

void task_exit(void) {
    __asm__ volatile("cli");
    printk("[Task] Task %d exited.\n", current_task->id);
    current_task->state = TASK_DEAD;
    __asm__ volatile("sti");

    schedule();
    while (1);
}

// 安全的 task_sleep
void task_sleep(uint32_t ticks) {
    if (ticks == 0) return;

    // 关中断，保护状态修改的原子性
    __asm__ volatile("cli");
    current_task->sleep_ticks = ticks;
    current_task->state = TASK_SLEEPING;

    // 立即触发调度 (schedule 内部会安全处理中断标志)
    schedule();
}

// 调度器核心
void schedule(void) {
    if (!current_task) return;

    // 禁用中断，防止调度过程中再次触发时钟中断导致死栈
    uint32_t eflags;
    __asm__ volatile("pushfl; popl %0; cli" : "=r"(eflags));

    task_t* prev = current_task;
    // 1. 扫描任务链表：更新睡眠 & 清理死亡任务
    task_t* p = task_chain;
    do {
        // 更新睡眠 Tick
        if (p->state == TASK_SLEEPING) {
            if (p->sleep_ticks > 0) {
                p->sleep_ticks--;
            }
            if (p->sleep_ticks == 0) {
                p->state = TASK_RUNNING; // 唤醒
            }
        }

        // 回收 DEAD 任务 (跳过当前正在运行的节点)
        if (p->next->state == TASK_DEAD && p->next != current_task) {
            task_t* dead = p->next;
            p->next = dead->next;
            if (task_chain == dead) task_chain = p;
            printk("[GC] Reclaimed Task ID: %d\n", dead->id);
            // 如果存在分配的内存 直接回收
            if (dead->stack) kfree(dead->stack);
            kfree(dead);
        }

        p = p->next;
    } while (p != task_chain);

    // 2. 寻找下一个 TASK_RUNNING 的任务
    task_t* next_to_run = 0;
    p = current_task->next;

    // 遍历一圈查找 RUNNING 任务
    for (int i = 0; i < 100; i++) { // 遍历安全上限
        if (p->state == TASK_RUNNING) {
            next_to_run = p;
            break;
        }
        p = p->next;
    }

    // 如果所有普通任务都在睡觉，回退选择 idle 任务 (PID 1)
    if (!next_to_run) {
        p = task_chain;
        do {
            if (p->id == 1) { // 找到 Idle 任务
                next_to_run = p;
                break;
            }
            p = p->next;
        } while (p != task_chain);
    }

    // 如果依然没找到，就保持当前任务
    if (!next_to_run) {
        next_to_run = current_task;
    }

    // 3. 执行上下文切换
    if (next_to_run != prev) {
        current_task = next_to_run;

        // 恢复之前的中断状态并切换
        if (eflags & 0x200) __asm__ volatile("sti");
        switch_to(prev, next_to_run);
    }
    else {
        // 未发生切换，恢复中断标记
        if (eflags & 0x200) __asm__ volatile("sti");
    }
}