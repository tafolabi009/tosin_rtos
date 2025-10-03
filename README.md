# Tosin RTOS - Real-Time Operating System

A lightweight, modular Real-Time Operating System (RTOS) designed for x86 architecture with support for embedded systems and microcontrollers.

## Overview

Tosin RTOS is a complete RTOS implementation featuring:
- **Bootloader (MBR)**: Custom x86 bootloader for kernel loading
- **Preemptive Scheduler**: Round-robin scheduling with 16 priority levels
- **Memory Management**: Efficient heap allocator with minimal overhead
- **IPC Mechanisms**: Semaphores and message queues for task synchronization
- **Basic Shell**: Interactive command-line interface
- **Modular Design**: Easy to extend with custom drivers and features

## Quick Start

### Build the OS

```bash
make
```

### Run in QEMU

```bash
make run
```

## Features

- ✅ Preemptive multitasking with round-robin scheduling
- ✅ 16 priority levels for task management
- ✅ Context switching optimized for x86
- ✅ Semaphores for synchronization
- ✅ Message queues for inter-task communication
- ✅ Simple heap memory allocator
- ✅ Interactive shell with built-in commands
- ✅ Modular kernel design with extensibility hooks
- ✅ Memory footprint optimized (~20KB kernel)

## Documentation

See [docs/README.md](docs/README.md) for comprehensive documentation including:
- Architecture overview
- API documentation
- Building and running instructions
- Programming guide
- Extension guide

## System Requirements

- NASM (Netwide Assembler)
- GCC (32-bit support)
- GNU ld (linker)
- GNU make
- QEMU (for testing)

## Shell Commands

Once booted, try these commands:
- `help` - List available commands
- `meminfo` - Display memory usage
- `ps` - Show process information
- `test` - Run task creation test
- `clear` - Clear the screen
- `uname` - System information

## Architecture

```
Bootloader (MBR) → Kernel → Scheduler → Tasks
                      ↓
                  Memory Manager
                      ↓
                  IPC (Semaphores/Queues)
                      ↓
                  Shell & Applications
```

## Memory Footprint

- Bootloader: 512 bytes
- Kernel: ~16-20 KB
- Per-task stack: 4 KB (configurable)
- Heap: 1 MB (configurable)

## License

This project is provided for educational and development purposes.

## Contributing

Contributions welcome! See documentation for extension guidelines
