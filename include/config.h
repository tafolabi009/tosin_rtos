#ifndef CONFIG_H
#define CONFIG_H

/* System Configuration */
#define MAX_TASKS           32      /* Maximum number of tasks */
#define TASK_STACK_SIZE     4096    /* Default task stack size */
#define TASK_NAME_LEN       32      /* Maximum task name length */

/* Scheduler Configuration */
#define TIMER_FREQ_HZ       100     /* System timer frequency (100Hz = 10ms) */
#define TIME_SLICE_MS       10      /* Time slice per task in ms */

/* Memory Configuration */
#define HEAP_SIZE           (1024 * 1024)  /* 1MB heap */
#define PAGE_SIZE           4096           /* Memory page size */

/* IPC Configuration */
#define MAX_SEMAPHORES      64      /* Maximum number of semaphores */
#define MAX_QUEUES          32      /* Maximum number of message queues */
#define QUEUE_SIZE          16      /* Default queue capacity */

/* Priority Levels */
#define PRIORITY_IDLE       0       /* Idle task priority */
#define PRIORITY_LOW        1       /* Low priority tasks */
#define PRIORITY_NORMAL     5       /* Normal priority tasks */
#define PRIORITY_HIGH       10      /* High priority tasks */
#define PRIORITY_CRITICAL   15      /* Critical priority tasks */
#define MAX_PRIORITY        15      /* Maximum priority level */

/* Shell Configuration */
#define SHELL_BUFFER_SIZE   256     /* Shell input buffer size */
#define SHELL_HISTORY_SIZE  10      /* Command history depth */

#endif /* CONFIG_H */
