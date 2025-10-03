#include "../include/shell.h"
#include "../include/io.h"
#include "../include/memory.h"
#include "../include/task.h"
#include "../include/scheduler.h"
#include "../include/config.h"

#define MAX_COMMANDS 32
#define MAX_ARGS 16

static shell_cmd_t commands[MAX_COMMANDS];
static int num_commands = 0;

/* Parse command line into arguments */
static int parse_command(char *line, char **argv) {
    int argc = 0;
    int in_arg = 0;
    
    while (*line && argc < MAX_ARGS) {
        if (*line == ' ' || *line == '\t') {
            if (in_arg) {
                *line = '\0';
                in_arg = 0;
            }
        } else {
            if (!in_arg) {
                argv[argc++] = line;
                in_arg = 1;
            }
        }
        line++;
    }
    
    return argc;
}

/* Built-in commands */
static int32_t cmd_help(int argc, char **argv) {
    int i;
    
    printf("Available commands:\n");
    for (i = 0; i < num_commands; i++) {
        printf("  %-15s - %s\n", commands[i].name, commands[i].help);
    }
    
    return SUCCESS;
}

static int32_t cmd_clear(int argc, char **argv) {
    int i;
    uint16_t *vga = (uint16_t *)0xB8000;
    
    for (i = 0; i < 80 * 25; i++) {
        vga[i] = 0x0F20;
    }
    
    return SUCCESS;
}

static int32_t cmd_meminfo(int argc, char **argv) {
    printf("Memory Information:\n");
    printf("  Total:  %u bytes\n", mem_get_total());
    printf("  Used:   %u bytes\n", mem_get_used());
    printf("  Free:   %u bytes\n", mem_get_free());
    
    return SUCCESS;
}

static int32_t cmd_ps(int argc, char **argv) {
    printf("Process Information:\n");
    printf("  Active tasks: %u\n", scheduler_get_task_count());
    printf("  System ticks: %u\n", scheduler_get_tick_count());
    
    return SUCCESS;
}

static int32_t cmd_echo(int argc, char **argv) {
    int i;
    
    for (i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) {
            putchar(' ');
        }
    }
    putchar('\n');
    
    return SUCCESS;
}

static int32_t cmd_uname(int argc, char **argv) {
    printf("Tosin RTOS v1.0\n");
    printf("Real-Time Operating System for x86\n");
    printf("Copyright (c) 2025\n");
    
    return SUCCESS;
}

/* Test task functions */
static void test_task_func(void *arg) {
    int count = 0;
    int id = (int)arg;
    
    while (count < 5) {
        printf("Task %d: iteration %d\n", id, count);
        count++;
        task_sleep(1000);
    }
    
    printf("Task %d: completed\n", id);
}

static int32_t cmd_test_tasks(int argc, char **argv) {
    task_t *task1, *task2;
    
    printf("Creating test tasks...\n");
    
    if (task_create(&task1, "test1", test_task_func, (void *)1, 
                    PRIORITY_NORMAL, 0) != SUCCESS) {
        printf("Failed to create task 1\n");
        return ERROR;
    }
    
    if (task_create(&task2, "test2", test_task_func, (void *)2, 
                    PRIORITY_NORMAL, 0) != SUCCESS) {
        printf("Failed to create task 2\n");
        return ERROR;
    }
    
    printf("Test tasks created successfully\n");
    
    return SUCCESS;
}

/* Initialize shell */
void shell_init(void) {
    num_commands = 0;
    
    /* Register built-in commands */
    shell_register_command("help", "Display available commands", cmd_help);
    shell_register_command("clear", "Clear the screen", cmd_clear);
    shell_register_command("meminfo", "Display memory information", cmd_meminfo);
    shell_register_command("ps", "Display process information", cmd_ps);
    shell_register_command("echo", "Echo arguments to output", cmd_echo);
    shell_register_command("uname", "Display system information", cmd_uname);
    shell_register_command("test", "Run task test", cmd_test_tasks);
}

/* Register a command */
int32_t shell_register_command(const char *name, const char *help, 
                               shell_cmd_handler_t handler) {
    if (num_commands >= MAX_COMMANDS) {
        return ERROR;
    }
    
    commands[num_commands].name = name;
    commands[num_commands].help = help;
    commands[num_commands].handler = handler;
    num_commands++;
    
    return SUCCESS;
}

/* Main shell loop */
void shell_run(void) {
    char buffer[SHELL_BUFFER_SIZE];
    char *argv[MAX_ARGS];
    int argc;
    int i;
    int found;
    
    printf("\n");
    printf("======================================\n");
    printf("   Tosin RTOS - Real-Time OS v1.0    \n");
    printf("======================================\n");
    printf("\n");
    printf("Type 'help' for available commands\n");
    printf("\n");
    
    while (1) {
        printf("rtos> ");
        
        if (gets(buffer, SHELL_BUFFER_SIZE) == 0) {
            continue;
        }
        
        argc = parse_command(buffer, argv);
        if (argc == 0) {
            continue;
        }
        
        found = 0;
        for (i = 0; i < num_commands; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].handler(argc, argv);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            printf("Unknown command: %s\n", argv[0]);
            printf("Type 'help' for available commands\n");
        }
    }
}
