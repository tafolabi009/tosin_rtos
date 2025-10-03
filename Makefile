# Makefile for Tosin RTOS

# Tools
AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

# Directories
BUILD_DIR = build
BOOT_DIR = boot
KERNEL_DIR = kernel
INCLUDE_DIR = include
LIB_DIR = lib
SHELL_DIR = shell

# Flags
ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -Wall -Wextra -I$(INCLUDE_DIR) -O2 -fno-pie -no-pie
LDFLAGS = -m elf_i386 -T $(KERNEL_DIR)/linker.ld

# Source files
BOOT_SRC = $(BOOT_DIR)/boot.asm
KERNEL_ASM_SRC = $(KERNEL_DIR)/core/entry.asm
KERNEL_C_SRC = $(KERNEL_DIR)/core/main.c \
               $(KERNEL_DIR)/core/task.c \
               $(KERNEL_DIR)/core/scheduler.c \
               $(KERNEL_DIR)/mm/memory.c \
               $(KERNEL_DIR)/ipc/semaphore.c \
               $(KERNEL_DIR)/ipc/queue.c
LIB_SRC = $(LIB_DIR)/io.c
SHELL_SRC = $(SHELL_DIR)/shell.c

# Object files
BOOT_OBJ = $(BUILD_DIR)/boot.bin
KERNEL_ASM_OBJ = $(BUILD_DIR)/entry.o
KERNEL_C_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(KERNEL_C_SRC)))
LIB_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(LIB_SRC)))
SHELL_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SHELL_SRC)))

KERNEL_OBJ = $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(LIB_OBJ) $(SHELL_OBJ)
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# Output
OS_IMAGE = $(BUILD_DIR)/tosin_rtos.img

# Default target
all: directories $(OS_IMAGE)

# Create directories
directories:
	@mkdir -p $(BUILD_DIR)

# Build bootloader
$(BOOT_OBJ): $(BOOT_SRC)
	@echo "Building bootloader..."
	$(AS) -f bin $(BOOT_SRC) -o $(BOOT_OBJ)

# Build kernel assembly
$(KERNEL_ASM_OBJ): $(KERNEL_ASM_SRC)
	@echo "Building kernel entry..."
	$(AS) $(ASFLAGS) $(KERNEL_ASM_SRC) -o $(KERNEL_ASM_OBJ)

# Build kernel C files
$(BUILD_DIR)/main.o: $(KERNEL_DIR)/core/main.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/task.o: $(KERNEL_DIR)/core/task.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/scheduler.o: $(KERNEL_DIR)/core/scheduler.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: $(KERNEL_DIR)/mm/memory.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/semaphore.o: $(KERNEL_DIR)/ipc/semaphore.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/queue.o: $(KERNEL_DIR)/ipc/queue.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/io.o: $(LIB_DIR)/io.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell.o: $(SHELL_DIR)/shell.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(BUILD_DIR)/kernel.elf: $(KERNEL_OBJ)
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) $(KERNEL_OBJ) -o $(BUILD_DIR)/kernel.elf

# Convert kernel to binary
$(KERNEL_BIN): $(BUILD_DIR)/kernel.elf
	@echo "Creating kernel binary..."
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(KERNEL_BIN)

# Create OS image
$(OS_IMAGE): $(BOOT_OBJ) $(KERNEL_BIN)
	@echo "Creating OS image..."
	@dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880 2>/dev/null
	@dd if=$(BOOT_OBJ) of=$(OS_IMAGE) conv=notrunc bs=512 count=1 2>/dev/null
	@dd if=$(KERNEL_BIN) of=$(OS_IMAGE) conv=notrunc bs=512 seek=1 2>/dev/null
	@echo "OS image created: $(OS_IMAGE)"
	@ls -lh $(OS_IMAGE)

# Run in QEMU
run: all
	qemu-system-i386 -fda $(OS_IMAGE)

# Run with debugging
debug: all
	qemu-system-i386 -fda $(OS_IMAGE) -s -S

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

# Help
help:
	@echo "Tosin RTOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the OS image (default)"
	@echo "  run     - Build and run in QEMU"
	@echo "  debug   - Build and run with GDB debugging"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"

.PHONY: all directories run debug clean help
