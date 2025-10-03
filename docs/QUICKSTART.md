# Tosin RTOS - Quick Start Guide

## 5-Minute Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install nasm gcc binutils make qemu-system-x86

# Fedora/RHEL
sudo dnf install nasm gcc binutils make qemu-system-x86

# Arch Linux
sudo pacman -S nasm gcc binutils make qemu
```

### Build & Run

```bash
# Clone repository (if needed)
git clone https://github.com/tafolabi009/tosin_rtos.git
cd tosin_rtos

# Build OS
make

# Run in QEMU
make run
```

You should see:
```
======================================
   Tosin RTOS - Real-Time OS v1.0    
======================================

Type 'help' for available commands

rtos>
```

### Try It Out

```bash
# In the RTOS shell, try these commands:
rtos> help
rtos> uname
rtos> meminfo
rtos> ps
rtos> test
rtos> clear
```

## Common Tasks

### Building from Scratch

```bash
make clean    # Remove all build artifacts
make          # Build everything
```

### Running Tests

```bash
# Start RTOS in QEMU
make run

# In shell, run test tasks
rtos> test

# You should see tasks creating and running
```

### Understanding Memory Usage

```bash
# In RTOS shell
rtos> meminfo

# Output:
# Memory Information:
#   Total:  1048576 bytes
#   Used:   xxxxx bytes
#   Free:   xxxxx bytes
```

### Checking Processes

```bash
# In RTOS shell
rtos> ps

# Output:
# Process Information:
#   Active tasks: 3
#   System ticks: 12345
```

## Programming Examples

### Example 1: Simple Task

```c
#include "task.h"
#include "io.h"

void my_task(void *arg) {
    int count = 0;
    
    while (1) {
        printf("Task running: %d\n", count++);
        task_sleep(1000);  // Sleep 1 second
    }
}

// Create task
task_t *task;
task_create(&task, "mytask", my_task, NULL, PRIORITY_NORMAL, 0);
```

### Example 2: Producer-Consumer

```c
#include "queue.h"
#include "task.h"

queue_t *msg_queue;

void producer(void *arg) {
    int data = 0;
    while (1) {
        queue_send(msg_queue, (void*)(long)data, 0);
        printf("Sent: %d\n", data);
        data++;
        task_sleep(500);
    }
}

void consumer(void *arg) {
    void *msg;
    while (1) {
        queue_receive(msg_queue, &msg, 0);
        printf("Received: %d\n", (int)(long)msg);
    }
}

// Setup
queue_create(&msg_queue, 10);
task_create(&prod, "producer", producer, NULL, PRIORITY_NORMAL, 0);
task_create(&cons, "consumer", consumer, NULL, PRIORITY_NORMAL, 0);
```

### Example 3: Using Semaphores

```c
#include "semaphore.h"
#include "task.h"

semaphore_t mutex;

void critical_task(void *arg) {
    int id = (int)(long)arg;
    
    while (1) {
        sem_wait(&mutex, 0);
        
        // Critical section
        printf("Task %d in critical section\n", id);
        task_sleep(100);
        
        sem_post(&mutex);
        task_sleep(500);
    }
}

// Setup
sem_init(&mutex, 1, 1);  // Binary semaphore
task_create(&t1, "task1", critical_task, (void*)1, PRIORITY_NORMAL, 0);
task_create(&t2, "task2", critical_task, (void*)2, PRIORITY_NORMAL, 0);
```

### Example 4: Custom Shell Command

```c
#include "shell.h"
#include "io.h"

int32_t cmd_hello(int argc, char **argv) {
    if (argc > 1) {
        printf("Hello, %s!\n", argv[1]);
    } else {
        printf("Hello, World!\n");
    }
    return SUCCESS;
}

// Register in shell_init()
shell_register_command("hello", "Say hello", cmd_hello);
```

## Configuration

Edit `include/config.h` to customize:

```c
// Increase maximum tasks
#define MAX_TASKS           64

// Change time slice
#define TIME_SLICE_MS       20

// Adjust heap size
#define HEAP_SIZE           (2 * 1024 * 1024)  // 2MB
```

## Troubleshooting

### Build Errors

**Problem:** `nasm: command not found`
```bash
sudo apt-get install nasm
```

**Problem:** `cannot find -lgcc`
```bash
sudo apt-get install gcc-multilib
```

**Problem:** `ld: cannot find -m elf_i386`
```bash
# Install 32-bit support
sudo apt-get install libc6-dev-i386
```

### Runtime Issues

**Problem:** QEMU not found
```bash
sudo apt-get install qemu-system-x86
```

**Problem:** Screen is blank
- Wait a few seconds for boot
- Try pressing Enter
- Check QEMU output for errors

**Problem:** Keyboard not working
- Click inside QEMU window
- Try different key combinations
- Check QEMU keyboard settings

## Development Workflow

### 1. Make Changes

```bash
# Edit source files
vim kernel/core/task.c

# Rebuild
make clean && make
```

### 2. Test Changes

```bash
# Run in QEMU
make run

# Test your changes
# Press Ctrl+A, then X to exit QEMU
```

### 3. Debug

```bash
# In terminal 1:
make debug

# In terminal 2:
gdb build/kernel.elf
(gdb) target remote localhost:1234
(gdb) break kmain
(gdb) continue
```

## Next Steps

1. **Read Documentation**
   - `docs/README.md` - Full documentation
   - `docs/API.md` - API reference
   - `docs/ARCHITECTURE.md` - Architecture details

2. **Try Examples**
   - Modify existing tasks
   - Create new shell commands
   - Experiment with priorities

3. **Extend the System**
   - Add device drivers
   - Implement file system
   - Port to other architectures

4. **Learn More**
   - Study the scheduler implementation
   - Understand context switching
   - Explore memory management

## Useful Commands Summary

```bash
# Build
make            # Build everything
make clean      # Clean build artifacts
./build.sh      # Build with verification

# Run
make run        # Run in QEMU
make debug      # Run with debugger

# Help
make help       # Show available targets
```

## RTOS Shell Commands Summary

```
help            List all commands
clear           Clear the screen
meminfo         Show memory usage
ps              Show process info
echo [args]     Echo arguments
uname           System information
test            Run task test
```

## Key Keyboard Shortcuts in QEMU

```
Ctrl+A, then X  Exit QEMU
Ctrl+A, then C  Switch to QEMU console
Ctrl+A, then 2  Switch to serial output
```

## Performance Tips

1. **Reduce Timer Frequency**
   - Lower `TIMER_FREQ_HZ` in `config.h`
   - Reduces scheduler overhead

2. **Optimize Task Priorities**
   - Use appropriate priorities
   - Avoid priority inversion

3. **Minimize Memory Allocation**
   - Reuse buffers
   - Pre-allocate at startup

4. **Use Efficient IPC**
   - Choose right primitive
   - Avoid busy-waiting

## Resources

- **Repository:** https://github.com/tafolabi009/tosin_rtos
- **Documentation:** See `docs/` directory
- **Issues:** Open GitHub issues for bugs
- **Discussions:** Use GitHub discussions

## Getting Help

1. Check documentation in `docs/`
2. Read the API reference
3. Look at example code
4. Search issues on GitHub
5. Ask in discussions

## Contributing

Contributions welcome! Areas:
- Additional device drivers
- Ports to other architectures
- Bug fixes
- Documentation improvements
- Example programs

---

**Happy coding with Tosin RTOS!** 🚀
