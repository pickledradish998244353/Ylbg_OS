; 功能：将新的 GDT 加载到 CPU，并刷新所有段寄存器
; 调用方式（C 语言）：gdt_flush((uint32_t)&gdtr);

[bits 32]

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]    ; eax = gdtr 的指针
    lgdt [eax]            ; 将 GDTR 加载到 CPU 的 GDTR 寄存器
    jmp 0x08:.reload_cs

.reload_cs:
    mov ax, 0x10
    mov ds, ax            ; 数据段
    mov es, ax            ; 附加段
    mov fs, ax            ; 附加段
    mov gs, ax            ; 附加段
    mov ss, ax            ; 栈段

    ret