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
# -m32                       : 生成 32 位代码
# -ffreestanding -nostdlib   : 裸机独立运行环境，不依赖宿主机标准库
# -fno-builtin               : 禁止编译器替换为内置库函数
# -fno-stack-protector       : 禁用金丝雀栈保护（避免触发未定义的 __stack_chk_fail）
# -fno-pic -fno-pie          : 彻底禁用位置无关代码（避免生成 GOT/PLT 表）
# -mno-sse -mno-sse2         : 禁用 SSE 指令集（解决 printk 可变参数崩溃的真凶！）
# -mno-mmx -mno-80387        : 禁用 MMX 和 x87 浮点指令
# -mpreferred-stack-boundary=2 : 强制栈按 4 字节对齐（默认 16 字节对齐会触发 SSE 向量指令）
set(CMAKE_C_FLAGS "-m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mpreferred-stack-boundary=2 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

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
)

# ==================== 【关键修改点 2：链接参数】 ====================
# 加上 -no-pie -static 保证生成纯粹的 32 位静态 ELF 内核
set_target_properties(kernel.bin PROPERTIES
        LINK_FLAGS "-m32 -Wl,-melf_i386 -T ${CMAKE_SOURCE_DIR}/linker.ld -nostdlib -no-pie -static"
        SUFFIX ""
)