# Tosin RTOS Architecture Guide

## System Overview

Tosin RTOS is a preemptive, priority-based real-time operating system designed for x86 architecture with a focus on modularity, low memory footprint, and extensibility.

## Boot Process

### 1. BIOS Stage
```
Power On → BIOS → Load MBR (Sector 0) → Jump to 0x7C00
```

### 2. Bootloader Stage (boot/boot.asm)
```
1. Setup segments (CS, DS, ES, SS)
2. Setup stack at 0x7C00
3. Display boot message
4. Load kernel from disk sectors 2-33 to 0x8000
5. Enable A20 line
6. Setup GDT (Global Descriptor Table)
7. Enter protected mode
8. Jump to kernel at 0x8000
```

### 3. Kernel Entry Stage (kernel/core/entry.asm)
```
1. Clear VGA screen
2. Setup IDT (Interrupt Descriptor Table)
3. Program PIC (Programmable Interrupt Controller)
4. Program PIT (Timer at 100Hz)
5. Call kmain()
```

### 4. Kernel Initialization (kernel/core/main.c)
```
1. Initialize memory manager with 1MB heap
2. Initialize scheduler
3. Create idle task (priority 0)
4. Create shell task (priority 5)
5. Start scheduler (enable interrupts)
```

## Memory Layout

```
0x00000000 - 0x000003FF   Real Mode IVT (Interrupt Vector Table)
0x00000400 - 0x000004FF   BIOS Data Area
0x00000500 - 0x00007BFF   Free conventional memory
0x00007C00 - 0x00007DFF   Bootloader (512 bytes)
0x00007E00 - 0x00007FFF   Bootloader stack
0x00008000 - 0x0000CFFF   Kernel code & data (~19KB)
0x0000D000 - 0x0008FFFF   Free memory
0x00090000 - 0x0009FFFF   Stack space (64KB)
0x000A0000 - 0x000BFFFF   Video memory
0x000B8000 - 0x000B8FA0   VGA text buffer (80x25)
0x000C0000 - 0x000FFFFF   BIOS ROM
0x00100000+               Extended memory (if available)

Heap: Static array in kernel (1MB by default)
Task Stacks: Dynamically allocated from heap (4KB each)
```

## Component Architecture

### 1. Task Management (kernel/core/task.c)

**Task Control Block (TCB):**
```c
struct task_struct {
    uint32_t task_id;           // Unique ID
    char name[32];              // Task name
    task_state_t state;         // READY/RUNNING/BLOCKED/SUSPENDED/TERMINATED
    uint8_t priority;           // 0-15 (higher = more important)
    uint32_t time_slice;        // Remaining time in ms
    cpu_context_t context;      // Saved registers
    uint32_t *stack_base;       // Stack memory
    uint32_t stack_size;        // Stack size
    struct task_struct *next;   // Queue linkage
    struct task_struct *prev;   // Queue linkage
    uint32_t wake_time;         // For task_sleep()
    void *wait_obj;             // IPC object waiting on
};
```

**Task States:**
```
      ┌─────────┐
      │ CREATED │
      └────┬────┘
           │ task_create()
           ▼
      ┌─────────┐
   ┌─▶│  READY  │◀─┐
   │  └────┬────┘  │
   │       │       │
   │       │ schedule()
   │       ▼       │
   │  ┌─────────┐ │
   │  │ RUNNING │ │ time_slice expired
   │  └────┬────┘ │ or task_yield()
   │       │       │
   │       └───────┘
   │       │
   │       │ sem_wait() / queue_receive()
   │       ▼
   │  ┌─────────┐
   └──│ BLOCKED │
      └────┬────┘
           │ task_exit()
           ▼
      ┌─────────┐
      │TERMINATED│
      └─────────┘
```

### 2. Scheduler (kernel/core/scheduler.c)

**Algorithm: Preemptive Round-Robin with Priorities**

```
Priority Queues:
┌──────────────────────────────────┐
│ Priority 15 (Critical)  → [T1]   │
│ Priority 14             → []     │
│ ...                              │
│ Priority 5  (Normal)    → [T2→T3]│
│ ...                              │
│ Priority 0  (Idle)      → [Idle] │
└──────────────────────────────────┘
```

**Scheduling Decision:**
1. Timer interrupt (every 10ms)
2. Decrement current task's time_slice
3. If time_slice == 0 or task yields:
   - Save current task context
   - Select highest priority ready task
   - Load new task context
   - Execute new task

**Context Switch Flow:**
```
Task A running
    │
    ▼
Timer interrupt
    │
    ▼
scheduler_tick_handler()
    │
    ▼
schedule()
    │
    ├─▶ Save Task A context
    │   (registers, flags, stack pointer)
    │
    ├─▶ Move Task A to ready queue
    │
    ├─▶ Select Task B (highest priority)
    │
    ├─▶ Load Task B context
    │
    └─▶ Return from interrupt
        │
        ▼
    Task B running
```

### 3. Memory Management (kernel/mm/memory.c)

**Heap Allocator: Best-Fit with Block Splitting**

```
Heap Structure:
┌─────────────────────────────────────────────┐
│ Block 1        │ Block 2        │ Block 3   │
│ [Header|Data]  │ [Header|Data]  │ [Header]  │
│ Size: 256      │ Size: 512      │ Size: 1024│
│ Free: No       │ Free: Yes      │ Free: Yes │
│ Next: Block2   │ Next: Block3   │ Next: NULL│
└─────────────────────────────────────────────┘
```

**Allocation Strategy:**
1. Search for smallest free block that fits
2. Split block if too large (>aligned_size + overhead)
3. Mark block as allocated
4. Return pointer to data area

**Deallocation Strategy:**
1. Mark block as free
2. Merge with adjacent free blocks (coalescing)

**Memory Overhead:**
- Per-block: 16 bytes header
- Alignment: 8 bytes
- Typical waste: <5% for mixed allocations

### 4. IPC - Semaphores (kernel/ipc/semaphore.c)

**Counting Semaphore:**
```c
struct semaphore {
    uint32_t count;         // Current count
    uint32_t max_count;     // Maximum count
    task_t *wait_queue;     // Blocked tasks
    bool_t valid;           // Is initialized
};
```

**Operations:**

**sem_wait() (P operation):**
```
if (count > 0) {
    count--;
    return SUCCESS;
} else {
    block current task;
    add to wait_queue;
    schedule();
}
```

**sem_post() (V operation):**
```
if (wait_queue not empty) {
    wake up first waiting task;
} else if (count < max_count) {
    count++;
}
```

**Use Cases:**
- Binary semaphore (1,1): Mutex for critical sections
- Counting semaphore: Resource pools
- Barrier synchronization: Task coordination

### 5. IPC - Message Queues (kernel/ipc/queue.c)

**Circular Buffer with Semaphores:**
```
Queue Structure:
┌───────────────────────────────────┐
│ Capacity: 16                      │
│ Count: 5                          │
│ Head: 2    Tail: 7               │
│                                   │
│ Buffer:                           │
│ [x][x][M1][M2][M3][M4][M5][_][_]  │
│        ▲                   ▲      │
│       Head                Tail    │
│                                   │
│ Semaphores:                       │
│ - mutex (1,1)        - mutual ex  │
│ - not_empty (5,16)   - reader sem │
│ - not_full (11,16)   - writer sem │
└───────────────────────────────────┘
```

**queue_send():**
```
sem_wait(not_full);      // Wait for space
sem_wait(mutex);         // Lock queue
buffer[tail] = msg;      // Add message
tail = (tail+1) % cap;   // Advance tail
count++;
sem_post(mutex);         // Unlock queue
sem_post(not_empty);     // Signal not empty
```

**queue_receive():**
```
sem_wait(not_empty);     // Wait for message
sem_wait(mutex);         // Lock queue
msg = buffer[head];      // Get message
head = (head+1) % cap;   // Advance head
count--;
sem_post(mutex);         // Unlock queue
sem_post(not_full);      // Signal not full
```

### 6. Shell (shell/shell.c)

**Command Processing:**
```
shell_run() loop:
    │
    ├─▶ Display prompt "rtos> "
    │
    ├─▶ Read line from keyboard
    │
    ├─▶ Parse into argc/argv
    │
    ├─▶ Search command table
    │
    ├─▶ Call handler(argc, argv)
    │
    └─▶ Repeat
```

**Built-in Commands:**
- `help`: List all commands
- `clear`: Clear screen
- `meminfo`: Show memory usage
- `ps`: Show task info
- `echo`: Echo arguments
- `uname`: System info
- `test`: Create test tasks

**Extensibility:**
```c
shell_register_command("mycmd", "Description", handler);
```

## Interrupt Handling

**IDT Setup:**
```
Entry 32 (IRQ0 - Timer): timer_interrupt_handler
   ↓
   ├─ Save all registers (pusha)
   ├─ Call scheduler_tick_handler()
   ├─ Send EOI to PIC
   ├─ Restore registers (popa)
   └─ iret
```

**PIC Configuration:**
```
Master PIC: IRQ0-7  → INT 0x20-0x27
Slave PIC:  IRQ8-15 → INT 0x28-0x2F
```

**Timer (PIT) Configuration:**
```
Frequency: 1193182 Hz (input)
Divisor: 11932
Output: 100 Hz (10ms period)
```

## Context Switch Details

**x86 Context:**
```c
struct cpu_context {
    uint32_t eax, ebx, ecx, edx;  // General purpose
    uint32_t esi, edi, ebp;        // Index/base
    uint32_t esp;                  // Stack pointer
    uint32_t eip;                  // Instruction pointer
    uint32_t eflags;               // Flags
    uint32_t cs, ss, ds, es, fs, gs; // Segments
};
```

**Context Switch Assembly (entry.asm):**
```nasm
context_switch(old_ctx, new_ctx):
    ; Save old context
    mov [old_ctx+EAX], eax
    mov [old_ctx+EBX], ebx
    ; ... (all registers)
    
    ; Restore new context
    mov eax, [new_ctx+EAX]
    mov ebx, [new_ctx+EBX]
    ; ... (all registers)
    
    jmp [new_ctx+EIP]  ; Jump to new task
```

## Performance Characteristics

**Context Switch:** ~100 CPU cycles (~0.1μs @ 1GHz)
**Scheduler Decision:** O(1) for priority selection, O(n) for round-robin within priority
**Memory Allocation:** O(n) where n = number of blocks (typically <100)
**Semaphore Operation:** O(1)
**Queue Operation:** O(1)

## Extensibility Points

### Adding Device Drivers
```c
// kernel/drivers/my_driver.c
void my_driver_init(void) {
    // Initialize hardware
}

void my_driver_read(void *buffer, size_t size) {
    // Read from device
}

// In kmain():
my_driver_init();
```

### Adding System Calls
```c
// Define new syscall
int32_t sys_my_syscall(int arg) {
    // Implementation
}

// Register with IDT
setup_syscall_gate(SYSCALL_NUM, sys_my_syscall);
```

### Adding Shell Commands
```c
int32_t my_command_handler(int argc, char **argv) {
    printf("Custom command!\n");
    return SUCCESS;
}

// In shell_init():
shell_register_command("mycommand", "My description", 
                      my_command_handler);
```

## Security Considerations

**Current Limitations:**
- No memory protection (no MMU/paging)
- All code runs in Ring 0 (kernel mode)
- No user/kernel space separation
- Tasks can access all memory

**Suitable For:**
- Single-application embedded systems
- Trusted code environments
- Educational purposes
- Prototyping

**Not Suitable For:**
- Multi-user systems
- Untrusted code execution
- Internet-facing systems

## Future Architecture Enhancements

1. **Memory Protection:**
   - Add paging for virtual memory
   - Separate kernel/user spaces
   - Per-task address spaces

2. **Multi-core Support:**
   - SMP scheduler
   - Per-CPU run queues
   - Spinlocks for synchronization

3. **File System:**
   - VFS layer
   - FAT16/32 support
   - Device file abstraction

4. **Network Stack:**
   - Ethernet driver
   - TCP/IP stack
   - Socket API

5. **Power Management:**
   - CPU frequency scaling
   - Idle power states
   - Device power management
