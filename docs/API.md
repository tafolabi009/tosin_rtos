# Tosin RTOS API Reference

## Task Management API

### task_create()
Create a new task.

```c
int32_t task_create(task_t **task, const char *name, task_func_t func, 
                    void *arg, uint8_t priority, uint32_t stack_size);
```

**Parameters:**
- `task` - Pointer to task pointer (output)
- `name` - Task name (max 32 characters)
- `func` - Task entry function
- `arg` - Argument passed to task function
- `priority` - Task priority (0-15, higher is better)
- `stack_size` - Stack size in bytes (0 for default)

**Returns:** SUCCESS or ERROR

### task_destroy()
Destroy a task.

```c
void task_destroy(task_t *task);
```

### task_yield()
Yield CPU to another task.

```c
void task_yield(void);
```

### task_sleep()
Sleep for specified milliseconds.

```c
void task_sleep(uint32_t ms);
```

### task_exit()
Exit current task.

```c
void task_exit(void);
```

### task_get_current()
Get current running task.

```c
task_t *task_get_current(void);
```

### task_set_priority()
Set task priority.

```c
int32_t task_set_priority(task_t *task, uint8_t priority);
```

## Scheduler API

### scheduler_init()
Initialize the scheduler.

```c
void scheduler_init(void);
```

### scheduler_start()
Start the scheduler.

```c
void scheduler_start(void);
```

### scheduler_get_tick_count()
Get system tick count.

```c
uint32_t scheduler_get_tick_count(void);
```

### scheduler_get_task_count()
Get number of active tasks.

```c
uint32_t scheduler_get_task_count(void);
```

### scheduler_disable_preemption()
Disable preemptive scheduling.

```c
void scheduler_disable_preemption(void);
```

### scheduler_enable_preemption()
Enable preemptive scheduling.

```c
void scheduler_enable_preemption(void);
```

## Memory Management API

### kmalloc()
Allocate memory from kernel heap.

```c
void *kmalloc(size_t size);
```

**Parameters:**
- `size` - Size in bytes to allocate

**Returns:** Pointer to allocated memory or NULL on failure

### kfree()
Free allocated memory.

```c
void kfree(void *ptr);
```

### krealloc()
Reallocate memory.

```c
void *krealloc(void *ptr, size_t new_size);
```

### mem_get_free()
Get free memory size.

```c
size_t mem_get_free(void);
```

### mem_get_used()
Get used memory size.

```c
size_t mem_get_used(void);
```

### mem_get_total()
Get total memory size.

```c
size_t mem_get_total(void);
```

### Memory Utilities

```c
void *memset(void *dest, int val, size_t len);
void *memcpy(void *dest, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);
```

## Semaphore API

### sem_init()
Initialize a semaphore.

```c
int32_t sem_init(semaphore_t *sem, uint32_t initial_count, uint32_t max_count);
```

**Parameters:**
- `sem` - Pointer to semaphore structure
- `initial_count` - Initial count value
- `max_count` - Maximum count value

**Returns:** SUCCESS or ERROR

### sem_wait()
Wait on a semaphore (P operation).

```c
int32_t sem_wait(semaphore_t *sem, uint32_t timeout_ms);
```

**Parameters:**
- `sem` - Pointer to semaphore
- `timeout_ms` - Timeout in milliseconds (0 = infinite)

**Returns:** SUCCESS or ERROR (timeout)

### sem_post()
Post to a semaphore (V operation).

```c
int32_t sem_post(semaphore_t *sem);
```

### sem_destroy()
Destroy a semaphore.

```c
int32_t sem_destroy(semaphore_t *sem);
```

### sem_get_count()
Get semaphore count.

```c
int32_t sem_get_count(semaphore_t *sem);
```

## Message Queue API

### queue_create()
Create a message queue.

```c
int32_t queue_create(queue_t **queue, uint32_t capacity);
```

**Parameters:**
- `queue` - Pointer to queue pointer (output)
- `capacity` - Maximum number of messages

**Returns:** SUCCESS or ERROR

### queue_send()
Send message to queue.

```c
int32_t queue_send(queue_t *queue, void *msg, uint32_t timeout_ms);
```

**Parameters:**
- `queue` - Pointer to queue
- `msg` - Message pointer
- `timeout_ms` - Timeout in milliseconds (0 = infinite)

**Returns:** SUCCESS or ERROR

### queue_receive()
Receive message from queue.

```c
int32_t queue_receive(queue_t *queue, void **msg, uint32_t timeout_ms);
```

**Parameters:**
- `queue` - Pointer to queue
- `msg` - Pointer to receive message
- `timeout_ms` - Timeout in milliseconds (0 = infinite)

**Returns:** SUCCESS or ERROR

### queue_destroy()
Destroy a queue.

```c
int32_t queue_destroy(queue_t *queue);
```

### queue_get_count()
Get number of messages in queue.

```c
uint32_t queue_get_count(queue_t *queue);
```

## I/O API

### Console Output

```c
void putchar(char c);              // Put single character
void puts(const char *s);           // Put string
void printf(const char *fmt, ...);  // Formatted output
```

**printf() format specifiers:**
- `%d`, `%i` - Signed integer
- `%u` - Unsigned integer
- `%x` - Hexadecimal
- `%s` - String
- `%c` - Character
- `%%` - Literal %

### Console Input

```c
char getchar(void);                 // Get single character
int gets(char *buf, int max_len);   // Get string
```

### String Utilities

```c
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int atoi(const char *str);
```

## Shell API

### shell_register_command()
Register a shell command.

```c
int32_t shell_register_command(const char *name, const char *help, 
                               shell_cmd_handler_t handler);
```

**Parameters:**
- `name` - Command name
- `help` - Help text
- `handler` - Command handler function

**Handler signature:**
```c
int32_t handler(int argc, char **argv);
```

## Priority Levels

```c
#define PRIORITY_IDLE       0   // Idle task
#define PRIORITY_LOW        1   // Low priority
#define PRIORITY_NORMAL     5   // Normal priority
#define PRIORITY_HIGH       10  // High priority
#define PRIORITY_CRITICAL   15  // Critical priority
#define MAX_PRIORITY        15  // Maximum
```

## Return Codes

```c
#define SUCCESS  0
#define ERROR   -1
```

## Example Usage

### Creating and Synchronizing Tasks

```c
#include "task.h"
#include "semaphore.h"

semaphore_t mutex;

void task1(void *arg) {
    while (1) {
        sem_wait(&mutex, 0);
        printf("Task 1 in critical section\n");
        task_sleep(100);
        sem_post(&mutex);
        task_sleep(500);
    }
}

void task2(void *arg) {
    while (1) {
        sem_wait(&mutex, 0);
        printf("Task 2 in critical section\n");
        task_sleep(100);
        sem_post(&mutex);
        task_sleep(500);
    }
}

void main_task(void *arg) {
    task_t *t1, *t2;
    
    // Initialize mutex
    sem_init(&mutex, 1, 1);
    
    // Create tasks
    task_create(&t1, "task1", task1, NULL, PRIORITY_NORMAL, 0);
    task_create(&t2, "task2", task2, NULL, PRIORITY_NORMAL, 0);
    
    // This task can now exit
    task_exit();
}
```

### Using Message Queues

```c
#include "queue.h"

queue_t *msg_queue;

void producer_task(void *arg) {
    int count = 0;
    
    while (1) {
        void *msg = (void *)(long)count;
        queue_send(msg_queue, msg, 0);
        printf("Sent: %d\n", count);
        count++;
        task_sleep(1000);
    }
}

void consumer_task(void *arg) {
    void *msg;
    
    while (1) {
        queue_receive(msg_queue, &msg, 0);
        printf("Received: %d\n", (int)(long)msg);
    }
}

void setup(void) {
    task_t *producer, *consumer;
    
    // Create queue
    queue_create(&msg_queue, 10);
    
    // Create tasks
    task_create(&producer, "producer", producer_task, NULL, 
                PRIORITY_NORMAL, 0);
    task_create(&consumer, "consumer", consumer_task, NULL, 
                PRIORITY_NORMAL, 0);
}
```
