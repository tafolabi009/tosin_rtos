#ifndef TASK_H
#define TASK_H

#include "types.h"
#include "config.h"

/* Task states */
typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED,
    TASK_TERMINATED
} task_state_t;

/* CPU context structure for x86 */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
} cpu_context_t;

/* Task Control Block (TCB) */
typedef struct task_struct {
    uint32_t task_id;                   /* Unique task ID */
    char name[TASK_NAME_LEN];           /* Task name */
    task_state_t state;                 /* Current state */
    uint8_t priority;                   /* Task priority (0-15) */
    uint32_t time_slice;                /* Remaining time slice */
    
    cpu_context_t context;              /* Saved CPU context */
    uint32_t *stack_base;               /* Stack base pointer */
    uint32_t stack_size;                /* Stack size */
    
    struct task_struct *next;           /* Next task in queue */
    struct task_struct *prev;           /* Previous task in queue */
    
    uint32_t wake_time;                 /* Wake time for sleeping tasks */
    void *wait_obj;                     /* Object task is waiting on */
} task_t;

/* Task function type */
typedef void (*task_func_t)(void *arg);

/* Task management functions */
int32_t task_create(task_t **task, const char *name, task_func_t func, 
                    void *arg, uint8_t priority, uint32_t stack_size);
void task_destroy(task_t *task);
void task_yield(void);
void task_sleep(uint32_t ms);
void task_exit(void);
task_t *task_get_current(void);
int32_t task_set_priority(task_t *task, uint8_t priority);

#endif /* TASK_H */
