#ifndef IO_H
#define IO_H

#include <stdint.h>

// 向指定端口写入一个字节
static inline void wb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// 从指定端口读取一个字节
static inline uint8_t rb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif