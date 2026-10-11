#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "printk.h"

typedef struct header {
    uint32_t size;        // 当前块的数据区大小 (字节)
    uint8_t  is_free;     // 1 表示空闲，0 表示已分配
    struct header* next;  // 指向下一个块头
} header_t;

#define HEAP_START 0xD0000000

static header_t* heap_first_block = nullptr;
static uint32_t ptr = HEAP_START;

// 扩容堆空间
static void heap_expand(size_t need) {
    uint32_t pages = (need + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t phys_page = pmm_alloc_page();
        if (!phys_page) {
            printk("[Heap Error] Out of physical memory for heap expansion!\n");
            return;
        }

        // 将虚拟页映射到物理页
        map_page(ptr, phys_page, VMM_PAGE_WRITABLE);
        ptr += PAGE_SIZE;
    }
}

void heap_init(void) {
    // 先分配一页
    heap_expand(PAGE_SIZE);
    heap_first_block = (header_t*)HEAP_START;
    heap_first_block->size = PAGE_SIZE - sizeof(header_t);
    heap_first_block->is_free = 1;
    heap_first_block->next = nullptr;
    printk("[Heap] Kernel Heap initialized at Virtual 0x%x\n", HEAP_START);
}

// 按字节分配内存
void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // 4 字节地址对齐
    size = (size + 3) & ~3;
    header_t* curr = heap_first_block;
    header_t* prev = nullptr;

    while (curr) {
        if (curr->is_free && curr->size >= size) {
            // 如果块太大，进行切分 (Split)
            if (curr->size >= size + sizeof(header_t) + 4) {
                header_t* new_block = (header_t*)((uint32_t)curr + sizeof(header_t) + size);
                new_block->size = curr->size - size - sizeof(header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }

            curr->is_free = 0; // 标记为已占用
            // 返回紧跟在 Header 后面的内存数据区指针！
            return (void*)((uint32_t)curr + sizeof(header_t));
        }
        prev = curr;
        curr = curr->next;
    }

    // 2. 如果已有的块都不够大，自动扩容堆空间！
    uint32_t need = size + sizeof(header_t);
    uint32_t old_max = ptr;
    heap_expand(need);

    // 在新扩容出来的空间上创建新块
    header_t* new_block = (header_t*)old_max;
    new_block->size = (ptr - old_max) - sizeof(header_t);
    new_block->is_free = 1;
    new_block->next = NULL;
    if (prev) {
        prev->next = new_block;
    }
    // 再次调用 kmalloc 分配
    return kmalloc(size);
}

void kfree(void* ptr) {
    if (!ptr) return;
    // 指针向前偏移一个 Header 大小，找到块头！
    header_t* header = (header_t*)((uint32_t)ptr - sizeof(header_t));
    header->is_free = 1;

    // 碎片合并 (Coalesce)：遍历堆，合并相邻的空闲块
    header_t* curr = heap_first_block;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += sizeof(header_t) + curr->next->size;
            curr->next = curr->next->next;
        }
        else {
            curr = curr->next;
        }
    }
}