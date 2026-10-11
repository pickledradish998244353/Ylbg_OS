[BITS 32]
section .text
global switch_to

switch_to:
    ; 此时栈上的布局 (进入函数前):
    ; [esp + 8] = next (目标任务指针)
    ; [esp + 4] = prev (当前任务指针)
    ; [esp + 0] = 返回地址 (EIP)

    ; 1. 保存当前任务 (prev) 的现场
    pushf
    pusha

    ; 2. 将当前的 ESP 保存到 prev->esp (offset 4)
    mov eax, [esp + 40]     ; eax = prev
    mov [eax + 4], esp      ; prev->esp = esp

    ; 切换到next 的esp 寄存器
    mov ebx, [esp + 44]     ; ebx = next
    mov esp, [ebx + 4]      ; esp = next->esp ! (CPU 瞬间穿越到了新任务的世界)

    ; 4. 恢复新任务 (next) 的现场
    popa                    ; 恢复通用寄存器
    popf                    ; 恢复 EFLAGS

    ret