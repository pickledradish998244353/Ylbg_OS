ENTRY(start)

SECTIONS
{
    . = 1M;

    /* 代码段 */
    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text*)
    }

    /* 只读数据段 */
    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    /* 可读写数据段 */
    .data : ALIGN(4K)
    {
        *(.data*)
    }

    /* 未初始化数据段 (BSS) */
    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss*)
    }
    end = .;
}