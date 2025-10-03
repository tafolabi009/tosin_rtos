#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory management functions */
void mem_init(void *heap_start, size_t heap_size);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

/* Memory utilities */
void *memset(void *dest, int val, size_t len);
void *memcpy(void *dest, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);

/* Memory statistics */
size_t mem_get_free(void);
size_t mem_get_used(void);
size_t mem_get_total(void);

#endif /* MEMORY_H */
