#include "../include/memory.h"

/* Simple block-based memory allocator */

typedef struct mem_block {
    size_t size;
    bool_t is_free;
    struct mem_block *next;
} mem_block_t;

static uint8_t *heap_start = NULL;
static size_t heap_size = 0;
static mem_block_t *free_list = NULL;
static size_t total_allocated = 0;

#define BLOCK_HEADER_SIZE sizeof(mem_block_t)
#define ALIGN_SIZE 8
#define ALIGN(size) (((size) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

/* Initialize memory manager */
void mem_init(void *heap_start_addr, size_t size) {
    heap_start = (uint8_t *)heap_start_addr;
    heap_size = size;
    
    /* Create initial free block */
    free_list = (mem_block_t *)heap_start;
    free_list->size = heap_size - BLOCK_HEADER_SIZE;
    free_list->is_free = TRUE;
    free_list->next = NULL;
    
    total_allocated = 0;
}

/* Find a free block that fits the requested size */
static mem_block_t *find_free_block(size_t size) {
    mem_block_t *current = free_list;
    mem_block_t *best_fit = NULL;
    size_t best_fit_size = (size_t)-1;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            if (current->size < best_fit_size) {
                best_fit = current;
                best_fit_size = current->size;
            }
        }
        current = current->next;
    }
    
    return best_fit;
}

/* Split a block if it's large enough */
static void split_block(mem_block_t *block, size_t size) {
    if (block->size >= size + BLOCK_HEADER_SIZE + ALIGN_SIZE) {
        mem_block_t *new_block = (mem_block_t *)((uint8_t *)block + BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->is_free = TRUE;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

/* Merge adjacent free blocks */
static void merge_free_blocks(void) {
    mem_block_t *current = free_list;
    
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            /* Check if blocks are adjacent */
            if ((uint8_t *)current + BLOCK_HEADER_SIZE + current->size == (uint8_t *)current->next) {
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        } else {
            current = current->next;
        }
    }
}

/* Allocate memory */
void *kmalloc(size_t size) {
    mem_block_t *block;
    
    if (size == 0 || !heap_start) {
        return NULL;
    }
    
    size = ALIGN(size);
    
    block = find_free_block(size);
    if (!block) {
        return NULL;
    }
    
    split_block(block, size);
    block->is_free = FALSE;
    total_allocated += size;
    
    return (void *)((uint8_t *)block + BLOCK_HEADER_SIZE);
}

/* Free memory */
void kfree(void *ptr) {
    mem_block_t *block;
    
    if (!ptr || !heap_start) {
        return;
    }
    
    block = (mem_block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    block->is_free = TRUE;
    
    if (total_allocated >= block->size) {
        total_allocated -= block->size;
    } else {
        total_allocated = 0;
    }
    
    merge_free_blocks();
}

/* Reallocate memory */
void *krealloc(void *ptr, size_t new_size) {
    void *new_ptr;
    mem_block_t *block;
    size_t old_size;
    size_t copy_size;
    
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    block = (mem_block_t *)((uint8_t *)ptr - BLOCK_HEADER_SIZE);
    old_size = block->size;
    
    if (new_size <= old_size) {
        return ptr;
    }
    
    new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    copy_size = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);
    
    return new_ptr;
}

/* Memory utilities */
void *memset(void *dest, int val, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    while (len--) {
        *d++ = (uint8_t)val;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t len) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (len--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t len) {
    const uint8_t *a = (const uint8_t *)s1;
    const uint8_t *b = (const uint8_t *)s2;
    while (len--) {
        if (*a != *b) {
            return *a - *b;
        }
        a++;
        b++;
    }
    return 0;
}

/* Memory statistics */
size_t mem_get_free(void) {
    mem_block_t *current = free_list;
    size_t free_size = 0;
    
    while (current) {
        if (current->is_free) {
            free_size += current->size;
        }
        current = current->next;
    }
    
    return free_size;
}

size_t mem_get_used(void) {
    return total_allocated;
}

size_t mem_get_total(void) {
    return heap_size;
}
