#include "../include/types.h"
#include "../include/config.h"
#include "../include/memory.h"
#include "../include/task.h"
#include "../include/scheduler.h"
#include "../include/shell.h"
#include "../include/io.h"

/* Heap memory area */
static uint8_t kernel_heap[HEAP_SIZE] __attribute__((aligned(PAGE_SIZE)));

/* Idle task function */
static void idle_task(void *arg) {
    while (1) {
        /* Halt CPU until next interrupt */
        __asm__ volatile("hlt");
    }
}

/* Shell task function */
static void shell_task(void *arg) {
    shell_init();
    shell_run();
}

/* Kernel main function */
void kmain(void) {
    task_t *idle;
    task_t *shell;
    
    /* Clear screen */
    printf("\n");
    printf("Tosin RTOS - Initializing...\n");
    printf("============================\n\n");
    
    /* Initialize memory management */
    printf("Initializing memory manager...\n");
    mem_init(kernel_heap, HEAP_SIZE);
    printf("  Heap size: %u bytes\n", HEAP_SIZE);
    
    /* Initialize scheduler */
    printf("Initializing scheduler...\n");
    scheduler_init();
    printf("  Timer frequency: %u Hz\n", TIMER_FREQ_HZ);
    printf("  Time slice: %u ms\n", TIME_SLICE_MS);
    
    /* Create idle task */
    printf("Creating idle task...\n");
    if (task_create(&idle, "idle", idle_task, NULL, PRIORITY_IDLE, 0) != SUCCESS) {
        printf("ERROR: Failed to create idle task!\n");
        while(1);
    }
    
    /* Create shell task */
    printf("Creating shell task...\n");
    if (task_create(&shell, "shell", shell_task, NULL, PRIORITY_NORMAL, 0) != SUCCESS) {
        printf("ERROR: Failed to create shell task!\n");
        while(1);
    }
    
    printf("\nInitialization complete!\n");
    printf("Starting scheduler...\n\n");
    
    /* Start scheduler - this should not return */
    scheduler_start();
    
    /* Should never reach here */
    printf("ERROR: Scheduler returned unexpectedly!\n");
    while(1);
}
