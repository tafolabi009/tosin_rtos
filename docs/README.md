# Tosin RTOS - Real-Time Operating System

A lightweight, modular Real-Time Operating System (RTOS) designed for x86 architecture with support for embedded systems and microcontrollers.

## Features

### Core Features
- **Bootloader (MBR)**: Custom x86 bootloader that loads the kernel from disk
- **Preemptive Multitasking**: Round-robin scheduling with priority levels (0-15)
- **Memory Management**: Simple heap allocator with best-fit strategy
- **Context Switching**: Fast assembly-based context switching for x86
- **IPC Mechanisms**: 
  - Semaphores for synchronization
  - Message queues for inter-task communication
- **Basic Shell**: Interactive command-line interface with built-in commands

### Design Philosophy
- **Modular Architecture**: Clean separation of concerns with extensibility hooks
- **Memory Optimized**: Minimal footprint suitable for resource-constrained systems
- **Portable Design**: Easy to adapt for different architectures

## System Architecture

```
┌─────────────────────────────────────────┐
│           Application Layer             │
│  (Shell, User Tasks, Custom Commands)   │
├─────────────────────────────────────────┤
│            Kernel Layer                 │
│  ┌──────────┬──────────┬──────────┐    │
│  │Scheduler │   IPC    │  Memory  │    │
│  │  (Core)  │(Queues/  │ Manager  │    │
│  │          │Semaphore)│          │    │
│  └──────────┴──────────┴──────────┘    │
├─────────────────────────────────────────┤
│         Hardware Abstraction            │
│  (I/O, Interrupts, Context Switch)      │
├─────────────────────────────────────────┤
│            Bootloader                   │
│        (MBR, 512 bytes)                 │
└─────────────────────────────────────────┘
```

## Directory Structure

```
tosin_rtos/
├── boot/               # Bootloader code
│   └── boot.asm       # MBR bootloader (x86 assembly)
├── kernel/            # Kernel code
│   ├── core/          # Core kernel functionality
│   │   ├── entry.asm  # Kernel entry and context switching
│   │   ├── main.c     # Kernel initialization
│   │   ├── task.c     # Task management
│   │   └── scheduler.c # Scheduler implementation
│   ├── mm/            # Memory management
│   │   └── memory.c   # Heap allocator
│   ├── ipc/           # Inter-process communication
│   │   ├── semaphore.c # Semaphore implementation
│   │   └── queue.c     # Message queue implementation
│   ├── drivers/       # Device drivers (extensible)
│   └── linker.ld      # Linker script
├── lib/               # Utility libraries
│   └── io.c           # I/O functions (printf, getchar, etc.)
├── shell/             # Command shell
│   └── shell.c        # Shell implementation
├── include/           # Header files
│   ├── types.h        # Basic type definitions
│   ├── config.h       # System configuration
│   ├── task.h         # Task management API
│   ├── scheduler.h    # Scheduler API
│   ├── memory.h       # Memory management API
│   ├── semaphore.h    # Semaphore API
│   ├── queue.h        # Queue API
│   ├── shell.h        # Shell API
│   └── io.h           # I/O API
├── build/             # Build artifacts
├── docs/              # Documentation
└── Makefile           # Build system
```

## Building

### Prerequisites
- NASM (Netwide Assembler)
- GCC (cross-compiler for i686-elf or native 32-bit)
- GNU ld (linker)
- GNU make

### Build Instructions

```bash
# Build the OS image
make

# Clean build artifacts
make clean

# View available targets
make help
```

The build process creates:
- `build/boot.bin` - Bootloader binary (512 bytes)
- `build/kernel.bin` - Kernel binary
- `build/tosin_rtos.img` - Complete OS image (floppy disk format)

## Running

### Using QEMU

```bash
# Run with default settings
make run

# Run with debugging support
make debug

# Manual invocation
qemu-system-i386 -fda build/tosin_rtos.img
```

### Using Real Hardware

You can write the OS image to a USB drive or floppy disk:

```bash
# WARNING: This will overwrite the target device!
sudo dd if=build/tosin_rtos.img of=/dev/sdX bs=512
```

Replace `/dev/sdX` with your target device.

## Usage

### Shell Commands

Once the system boots, you'll see a command prompt:

```
rtos>
```

Available commands:
- `help` - Display available commands
- `clear` - Clear the screen
- `meminfo` - Display memory statistics
- `ps` - Display process information
- `echo [args]` - Echo arguments to output
- `uname` - Display system information
- `test` - Run task creation test

### Programming with Tosin RTOS

#### Creating a Task

```c
#include "task.h"

void my_task(void *arg) {
    while (1) {
        printf("Task running!\n");
        task_sleep(1000);  // Sleep 1 second
    }
}

// Create task with normal priority
task_t *task;
task_create(&task, "my_task", my_task, NULL, PRIORITY_NORMAL, 0);
```

#### Using Semaphores

```c
#include "semaphore.h"

semaphore_t sem;

// Initialize binary semaphore
sem_init(&sem, 1, 1);

// Wait (P operation)
sem_wait(&sem, 0);  // 0 = no timeout

// Critical section
// ...

// Signal (V operation)
sem_post(&sem);
```

#### Using Message Queues

```c
#include "queue.h"

queue_t *queue;

// Create queue with capacity of 10
queue_create(&queue, 10);

// Send message
void *msg = "Hello";
queue_send(queue, msg, 0);

// Receive message
void *received;
queue_receive(queue, &received, 0);
```

## Configuration

Edit `include/config.h` to customize:

- `MAX_TASKS` - Maximum number of tasks
- `TASK_STACK_SIZE` - Default stack size per task
- `TIMER_FREQ_HZ` - System timer frequency
- `TIME_SLICE_MS` - Time slice per task
- `HEAP_SIZE` - Total heap memory
- Priority levels and other parameters

## Extending the System

### Adding Device Drivers

1. Create driver file in `kernel/drivers/`
2. Define initialization function
3. Register driver hooks in kernel initialization
4. Implement driver-specific functionality

### Adding Shell Commands

```c
#include "shell.h"

int32_t my_command(int argc, char **argv) {
    printf("My custom command!\n");
    return SUCCESS;
}

// Register in shell_init()
shell_register_command("mycommand", "My custom command", my_command);
```

## Memory Footprint

Typical memory usage:
- Bootloader: 512 bytes
- Kernel code: ~16-20 KB
- Heap: Configurable (default 1 MB)
- Per-task stack: Configurable (default 4 KB)

Total minimal footprint: ~20 KB + heap + task stacks

## Technical Details

### Scheduler
- **Algorithm**: Preemptive round-robin with priority queues
- **Priority Levels**: 16 levels (0 = idle, 15 = highest)
- **Time Slice**: Configurable (default 10ms)
- **Context Switch**: Assembly-optimized for x86

### Memory Management
- **Allocator**: Best-fit heap allocator
- **Alignment**: 8-byte alignment for efficiency
- **Features**: Block splitting and merging
- **Overhead**: ~16 bytes per allocation

### IPC
- **Semaphores**: Counting semaphores with priority inheritance
- **Queues**: Fixed-size circular buffers with semaphore synchronization
- **Thread Safety**: All IPC primitives are interrupt-safe

## Limitations

- x86 32-bit only (no 64-bit support)
- No virtual memory or MMU support
- No file system
- No network stack
- Limited device driver support
- Single-core only

## Future Enhancements

- [ ] Port to ARM Cortex-M
- [ ] Add RISC-V support
- [ ] Implement simple file system
- [ ] Add more device drivers
- [ ] Power management features
- [ ] Multi-core support
- [ ] Dynamic priority adjustment

## License

This project is provided as-is for educational and development purposes.

## Contributing

Contributions are welcome! Areas for improvement:
- Additional device drivers
- Ports to other architectures
- Bug fixes and optimizations
- Documentation improvements

## Contact

For questions and support, please open an issue on the repository.

## Acknowledgments

Built with modern operating system design principles, inspired by classic RTOS implementations and educational operating systems.
