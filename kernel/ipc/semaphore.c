#include "../include/semaphore.h"
#include "../include/scheduler.h"
#include "../include/memory.h"

/* Add task to semaphore wait queue */
static void sem_add_waiter(semaphore_t *sem, task_t *task) {
    if (!sem->wait_queue) {
        sem->wait_queue = task;
        task->next = task;
        task->prev = task;
    } else {
        task->next = sem->wait_queue;
        task->prev = sem->wait_queue->prev;
        sem->wait_queue->prev->next = task;
        sem->wait_queue->prev = task;
    }
}

/* Remove task from semaphore wait queue */
static task_t *sem_remove_waiter(semaphore_t *sem) {
    task_t *task;
    
    if (!sem->wait_queue) {
        return NULL;
    }
    
    task = sem->wait_queue;
    
    if (task->next == task) {
        sem->wait_queue = NULL;
    } else {
        sem->wait_queue = task->next;
        task->prev->next = task->next;
        task->next->prev = task->prev;
    }
    
    task->next = NULL;
    task->prev = NULL;
    
    return task;
}

/* Initialize a semaphore */
int32_t sem_init(semaphore_t *sem, uint32_t initial_count, uint32_t max_count) {
    if (!sem || initial_count > max_count) {
        return ERROR;
    }
    
    sem->count = initial_count;
    sem->max_count = max_count;
    sem->wait_queue = NULL;
    sem->valid = TRUE;
    
    return SUCCESS;
}

/* Wait on a semaphore (P operation) */
int32_t sem_wait(semaphore_t *sem, uint32_t timeout_ms) {
    task_t *current;
    uint32_t start_time;
    
    if (!sem || !sem->valid) {
        return ERROR;
    }
    
    scheduler_disable_preemption();
    
    if (sem->count > 0) {
        sem->count--;
        scheduler_enable_preemption();
        return SUCCESS;
    }
    
    /* Need to wait */
    current = task_get_current();
    if (!current) {
        scheduler_enable_preemption();
        return ERROR;
    }
    
    /* Add to wait queue */
    sem_add_waiter(sem, current);
    current->wait_obj = sem;
    
    if (timeout_ms > 0) {
        start_time = scheduler_get_tick_count();
        current->wake_time = start_time + (timeout_ms * TIMER_FREQ_HZ) / 1000;
    }
    
    scheduler_enable_preemption();
    scheduler_block_task(current);
    schedule();
    
    /* Check if timed out */
    if (timeout_ms > 0 && current->wait_obj == sem) {
        /* Timed out, remove from wait queue */
        scheduler_disable_preemption();
        if (current->next) {
            if (current->next == current) {
                sem->wait_queue = NULL;
            } else {
                if (sem->wait_queue == current) {
                    sem->wait_queue = current->next;
                }
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            current->next = NULL;
            current->prev = NULL;
        }
        current->wait_obj = NULL;
        scheduler_enable_preemption();
        return ERROR;
    }
    
    return SUCCESS;
}

/* Post to a semaphore (V operation) */
int32_t sem_post(semaphore_t *sem) {
    task_t *task;
    
    if (!sem || !sem->valid) {
        return ERROR;
    }
    
    scheduler_disable_preemption();
    
    if (sem->wait_queue) {
        /* Wake up a waiting task */
        task = sem_remove_waiter(sem);
        if (task) {
            task->wait_obj = NULL;
            task->wake_time = 0;
            scheduler_unblock_task(task);
        }
    } else if (sem->count < sem->max_count) {
        sem->count++;
    }
    
    scheduler_enable_preemption();
    
    return SUCCESS;
}

/* Destroy a semaphore */
int32_t sem_destroy(semaphore_t *sem) {
    task_t *task;
    
    if (!sem || !sem->valid) {
        return ERROR;
    }
    
    scheduler_disable_preemption();
    
    /* Wake up all waiting tasks */
    while (sem->wait_queue) {
        task = sem_remove_waiter(sem);
        if (task) {
            task->wait_obj = NULL;
            scheduler_unblock_task(task);
        }
    }
    
    sem->valid = FALSE;
    scheduler_enable_preemption();
    
    return SUCCESS;
}

/* Get semaphore count */
int32_t sem_get_count(semaphore_t *sem) {
    if (!sem || !sem->valid) {
        return ERROR;
    }
    
    return (int32_t)sem->count;
}
