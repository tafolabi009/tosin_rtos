#include "../include/scheduler.h"
#include "../include/task.h"
#include "../include/memory.h"
#include "../include/io.h"

/* External assembly functions */
extern void context_switch(cpu_context_t *old_ctx, cpu_context_t *new_ctx);
extern void setup_idt(void);
extern void enable_interrupts(void);
extern void disable_interrupts(void);

/* External task function */
extern void task_set_current(task_t *task);

/* Ready queue per priority level */
static task_t *ready_queue[MAX_PRIORITY + 1];
static task_t *blocked_queue = NULL;
static task_t *current_task = NULL;

static uint32_t tick_count = 0;
static uint32_t task_count = 0;
static bool_t preemption_enabled = TRUE;
static bool_t scheduler_running = FALSE;

/* Add task to end of priority queue */
static void add_to_queue(task_t **queue, task_t *task) {
    if (!*queue) {
        *queue = task;
        task->next = task;
        task->prev = task;
    } else {
        task->next = *queue;
        task->prev = (*queue)->prev;
        (*queue)->prev->next = task;
        (*queue)->prev = task;
    }
}

/* Remove task from queue */
static void remove_from_queue(task_t **queue, task_t *task) {
    if (!*queue || !task) {
        return;
    }
    
    if (task->next == task) {
        /* Only task in queue */
        *queue = NULL;
    } else {
        if (*queue == task) {
            *queue = task->next;
        }
        task->prev->next = task->next;
        task->next->prev = task->prev;
    }
    
    task->next = NULL;
    task->prev = NULL;
}

/* Get next ready task (highest priority, round-robin within priority) */
static task_t *get_next_task(void) {
    int32_t i;
    task_t *task;
    
    for (i = MAX_PRIORITY; i >= 0; i--) {
        if (ready_queue[i]) {
            task = ready_queue[i];
            ready_queue[i] = task->next;  /* Round-robin */
            return task;
        }
    }
    
    return NULL;
}

/* Initialize scheduler */
void scheduler_init(void) {
    int32_t i;
    
    for (i = 0; i <= MAX_PRIORITY; i++) {
        ready_queue[i] = NULL;
    }
    
    blocked_queue = NULL;
    current_task = NULL;
    tick_count = 0;
    task_count = 0;
    preemption_enabled = TRUE;
    scheduler_running = FALSE;
    
    /* Setup interrupt descriptor table */
    setup_idt();
}

/* Start scheduler (enables interrupts) */
void scheduler_start(void) {
    scheduler_running = TRUE;
    enable_interrupts();
    
    /* Initial schedule */
    schedule();
}

/* Timer tick handler */
void scheduler_tick_handler(void) {
    task_t *task;
    task_t *next;
    
    tick_count++;
    
    /* Wake up sleeping tasks */
    task = blocked_queue;
    if (task) {
        do {
            next = task->next;
            if (task->wake_time > 0 && tick_count >= task->wake_time) {
                task->wake_time = 0;
                scheduler_unblock_task(task);
            }
            task = next;
        } while (task != blocked_queue && task);
    }
    
    /* Decrement time slice of current task */
    if (current_task && preemption_enabled) {
        if (current_task->time_slice > 0) {
            current_task->time_slice--;
        }
        
        if (current_task->time_slice == 0) {
            /* Time slice expired, trigger reschedule */
            schedule();
        }
    }
}

/* Main scheduling function */
void schedule(void) {
    task_t *old_task;
    task_t *new_task;
    
    if (!scheduler_running) {
        return;
    }
    
    disable_interrupts();
    
    old_task = current_task;
    
    /* Put current task back in ready queue if still runnable */
    if (old_task && old_task->state == TASK_RUNNING) {
        old_task->state = TASK_READY;
        old_task->time_slice = TIME_SLICE_MS;
        add_to_queue(&ready_queue[old_task->priority], old_task);
    }
    
    /* Get next task to run */
    new_task = get_next_task();
    
    if (new_task) {
        new_task->state = TASK_RUNNING;
        new_task->time_slice = TIME_SLICE_MS;
        current_task = new_task;
        task_set_current(new_task);
        
        /* Context switch */
        if (old_task != new_task) {
            if (old_task) {
                context_switch(&old_task->context, &new_task->context);
            } else {
                context_switch(NULL, &new_task->context);
            }
        }
    } else {
        /* No task to run, idle */
        current_task = NULL;
        task_set_current(NULL);
    }
    
    enable_interrupts();
}

/* Add task to ready queue */
void scheduler_add_task(task_t *task) {
    if (!task) {
        return;
    }
    
    disable_interrupts();
    task->state = TASK_READY;
    add_to_queue(&ready_queue[task->priority], task);
    task_count++;
    enable_interrupts();
}

/* Remove task from scheduler */
void scheduler_remove_task(task_t *task) {
    int32_t i;
    
    if (!task) {
        return;
    }
    
    disable_interrupts();
    
    /* Remove from ready queue */
    for (i = 0; i <= MAX_PRIORITY; i++) {
        if (ready_queue[i]) {
            remove_from_queue(&ready_queue[i], task);
        }
    }
    
    /* Remove from blocked queue */
    remove_from_queue(&blocked_queue, task);
    
    if (task_count > 0) {
        task_count--;
    }
    
    enable_interrupts();
}

/* Block a task */
void scheduler_block_task(task_t *task) {
    if (!task) {
        return;
    }
    
    disable_interrupts();
    task->state = TASK_BLOCKED;
    
    /* Remove from ready queue */
    remove_from_queue(&ready_queue[task->priority], task);
    
    /* Add to blocked queue */
    add_to_queue(&blocked_queue, task);
    
    enable_interrupts();
}

/* Unblock a task */
void scheduler_unblock_task(task_t *task) {
    if (!task) {
        return;
    }
    
    disable_interrupts();
    
    /* Remove from blocked queue */
    remove_from_queue(&blocked_queue, task);
    
    /* Add to ready queue */
    task->state = TASK_READY;
    add_to_queue(&ready_queue[task->priority], task);
    
    enable_interrupts();
}

/* Disable preemption */
void scheduler_disable_preemption(void) {
    preemption_enabled = FALSE;
}

/* Enable preemption */
void scheduler_enable_preemption(void) {
    preemption_enabled = TRUE;
}

/* Get tick count */
uint32_t scheduler_get_tick_count(void) {
    return tick_count;
}

/* Get task count */
uint32_t scheduler_get_task_count(void) {
    return task_count;
}
