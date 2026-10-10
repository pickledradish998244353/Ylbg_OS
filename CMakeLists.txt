cmake_minimum_required(VERSION 3.20)

# 【关键】强制彻底禁用 macOS 的所有默认行为
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)
set(CMAKE_OSX_ARCHITECTURES "") # 禁用 -arch arm64
set(CMAKE_OSX_SYSROOT "")       # 禁用 SDK 检测
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY) # 禁止尝试链接

# 【关键】强制锁定交叉编译器路径
set(CMAKE_C_COMPILER /opt/homebrew/bin/x86_64-elf-gcc)
set(CMAKE_ASM_NASM_COMPILER /opt/homebrew/bin/nasm)

project(MyOS C ASM_NASM)

# 强制 NASM 的输出格式为 elf32
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> -f elf32 <FLAGS> -o <OBJECT> <SOURCE>")

# ==================== 【关键修改点 1：C 编译参数】 ====================
set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mpreferred-stack-boundary=2 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# ==================== 【注意这里的顺序！】 ====================
# kernel.bin 是生成的目标文件
# boot.asm 是第一个源文件（包含 Multiboot Header）
add_executable(kernel.bin
        boot.asm
        kernel.c
        vga.c
        vga.h
        io.h
        printk.h
        printk.c
        gdt.h
        gdt.c
        gdt_flush.asm
        idt.h
        idt.c
        idt_asm.asm
        timer.h
        timer.c
        keyboard.h
        keyboard.c
        multiboot.h
        pmm.h
        pmm.c
        vmm.h
        vmm.c
)

# ==================== 【关键修改点 2：链接参数】 ====================
set_target_properties(kernel.bin PROPERTIES
        LINK_FLAGS "-m32 -Wl,-melf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -no-pie -static"
        SUFFIX ""
)