#include "../include/task.h"
#include "../include/memory.h"
#include "../include/scheduler.h"
#include "../include/io.h"

static uint32_t next_task_id = 1;
static task_t *current_task = NULL;

/* Task wrapper function that calls the actual task and handles exit */
static void task_wrapper(task_func_t func, void *arg) {
    func(arg);
    task_exit();
}

/* Create a new task */
int32_t task_create(task_t **task, const char *name, task_func_t func, 
                    void *arg, uint8_t priority, uint32_t stack_size) {
    task_t *new_task;
    uint32_t *stack;
    uint32_t i;
    
    if (!task || !name || !func || priority > MAX_PRIORITY) {
        return ERROR;
    }
    
    if (stack_size == 0) {
        stack_size = TASK_STACK_SIZE;
    }
    
    /* Allocate task control block */
    new_task = (task_t *)kmalloc(sizeof(task_t));
    if (!new_task) {
        return ERROR;
    }
    
    /* Allocate stack */
    stack = (uint32_t *)kmalloc(stack_size);
    if (!stack) {
        kfree(new_task);
        return ERROR;
    }
    
    /* Initialize task control block */
    new_task->task_id = next_task_id++;
    for (i = 0; i < TASK_NAME_LEN - 1 && name[i]; i++) {
        new_task->name[i] = name[i];
    }
    new_task->name[i] = '\0';
    
    new_task->state = TASK_READY;
    new_task->priority = priority;
    new_task->time_slice = TIME_SLICE_MS;
    
    new_task->stack_base = stack;
    new_task->stack_size = stack_size;
    
    new_task->next = NULL;
    new_task->prev = NULL;
    new_task->wake_time = 0;
    new_task->wait_obj = NULL;
    
    /* Setup initial stack frame for context switching */
    stack = (uint32_t *)((uint32_t)stack + stack_size);
    
    /* Push initial context onto stack */
    *(--stack) = (uint32_t)arg;              /* Argument */
    *(--stack) = (uint32_t)func;             /* Function pointer */
    *(--stack) = 0;                          /* Return address (unused) */
    *(--stack) = (uint32_t)task_wrapper;     /* EIP - task wrapper */
    *(--stack) = 0x00000202;                 /* EFLAGS - IF set */
    *(--stack) = 0;                          /* EAX */
    *(--stack) = 0;                          /* EBX */
    *(--stack) = 0;                          /* ECX */
    *(--stack) = 0;                          /* EDX */
    *(--stack) = 0;                          /* ESI */
    *(--stack) = 0;                          /* EDI */
    *(--stack) = 0;                          /* EBP */
    
    /* Initialize CPU context */
    new_task->context.esp = (uint32_t)stack;
    new_task->context.eip = (uint32_t)task_wrapper;
    new_task->context.eflags = 0x00000202;   /* IF set */
    new_task->context.cs = 0x08;             /* Kernel code segment */
    new_task->context.ss = 0x10;             /* Kernel data segment */
    new_task->context.ds = 0x10;
    new_task->context.es = 0x10;
    new_task->context.fs = 0x10;
    new_task->context.gs = 0x10;
    
    *task = new_task;
    
    /* Add task to scheduler */
    scheduler_add_task(new_task);
    
    return SUCCESS;
}

/* Destroy a task */
void task_destroy(task_t *task) {
    if (!task) {
        return;
    }
    
    scheduler_remove_task(task);
    
    if (task->stack_base) {
        kfree(task->stack_base);
    }
    
    kfree(task);
}

/* Yield CPU to another task */
void task_yield(void) {
    schedule();
}

/* Sleep for specified milliseconds */
void task_sleep(uint32_t ms) {
    task_t *task = task_get_current();
    if (task) {
        task->wake_time = scheduler_get_tick_count() + (ms * TIMER_FREQ_HZ) / 1000;
        scheduler_block_task(task);
        schedule();
    }
}

/* Exit current task */
void task_exit(void) {
    task_t *task = task_get_current();
    if (task) {
        task->state = TASK_TERMINATED;
        schedule();
    }
    
    /* Should never reach here */
    while(1);
}

/* Get current running task */
task_t *task_get_current(void) {
    return current_task;
}

/* Set current task (called by scheduler) */
void task_set_current(task_t *task) {
    current_task = task;
}

/* Set task priority */
int32_t task_set_priority(task_t *task, uint8_t priority) {
    if (!task || priority > MAX_PRIORITY) {
        return ERROR;
    }
    
    task->priority = priority;
    return SUCCESS;
}
