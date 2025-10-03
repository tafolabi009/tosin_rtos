#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "types.h"
#include "task.h"

/* Semaphore structure */
typedef struct {
    uint32_t count;         /* Current count */
    uint32_t max_count;     /* Maximum count */
    task_t *wait_queue;     /* Queue of waiting tasks */
    bool_t valid;           /* Semaphore is valid */
} semaphore_t;

/* Semaphore operations */
int32_t sem_init(semaphore_t *sem, uint32_t initial_count, uint32_t max_count);
int32_t sem_wait(semaphore_t *sem, uint32_t timeout_ms);
int32_t sem_post(semaphore_t *sem);
int32_t sem_destroy(semaphore_t *sem);
int32_t sem_get_count(semaphore_t *sem);

#endif /* SEMAPHORE_H */
