#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "task.h"

/* Scheduler initialization and control */
void scheduler_init(void);
void scheduler_start(void);
void scheduler_tick(void);
void schedule(void);

/* Task queue management */
void scheduler_add_task(task_t *task);
void scheduler_remove_task(task_t *task);
void scheduler_block_task(task_t *task);
void scheduler_unblock_task(task_t *task);

/* Preemption control */
void scheduler_disable_preemption(void);
void scheduler_enable_preemption(void);

/* Statistics */
uint32_t scheduler_get_tick_count(void);
uint32_t scheduler_get_task_count(void);

#endif /* SCHEDULER_H */
