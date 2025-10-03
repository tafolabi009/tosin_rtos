#!/bin/bash
# Build and Test Script for Tosin RTOS

set -e

echo "================================================"
echo "      Tosin RTOS - Build & Test Script         "
echo "================================================"
echo

# Check for required tools
echo "Checking for required build tools..."
MISSING_TOOLS=0

check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "  ✗ $1 is not installed"
        MISSING_TOOLS=1
    else
        echo "  ✓ $1 found"
    fi
}

check_tool nasm
check_tool gcc
check_tool ld
check_tool objcopy
check_tool make

if [ $MISSING_TOOLS -eq 1 ]; then
    echo
    echo "Error: Missing required tools. Please install them first."
    echo "On Ubuntu/Debian: sudo apt-get install nasm gcc binutils make"
    exit 1
fi

echo
echo "All required tools are available!"
echo

# Clean previous build
echo "Cleaning previous build..."
make clean

echo
echo "Building Tosin RTOS..."
echo

# Build
make

echo
echo "================================================"
echo "             Build Summary                      "
echo "================================================"

# Check build artifacts
if [ -f "build/boot.bin" ]; then
    BOOT_SIZE=$(stat -c%s build/boot.bin)
    echo "✓ Bootloader: build/boot.bin ($BOOT_SIZE bytes)"
    if [ $BOOT_SIZE -eq 512 ]; then
        echo "  ✓ Bootloader size is correct (512 bytes)"
    else
        echo "  ✗ Warning: Bootloader size is not 512 bytes!"
    fi
else
    echo "✗ Bootloader build failed!"
    exit 1
fi

if [ -f "build/kernel.bin" ]; then
    KERNEL_SIZE=$(stat -c%s build/kernel.bin)
    KERNEL_SIZE_KB=$((KERNEL_SIZE / 1024))
    echo "✓ Kernel: build/kernel.bin ($KERNEL_SIZE_KB KB)"
else
    echo "✗ Kernel build failed!"
    exit 1
fi

if [ -f "build/tosin_rtos.img" ]; then
    IMG_SIZE=$(stat -c%s build/tosin_rtos.img)
    IMG_SIZE_KB=$((IMG_SIZE / 1024))
    echo "✓ OS Image: build/tosin_rtos.img ($IMG_SIZE_KB KB)"
else
    echo "✗ OS image creation failed!"
    exit 1
fi

# Verify boot signature
BOOT_SIG=$(xxd -s 510 -l 2 -p build/boot.bin)
if [ "$BOOT_SIG" == "55aa" ]; then
    echo "✓ Boot signature is valid (0x55AA)"
else
    echo "✗ Warning: Boot signature is invalid ($BOOT_SIG)"
fi

echo
echo "================================================"
echo "           Build Successful!                    "
echo "================================================"
echo
echo "To run in QEMU:"
echo "  make run"
echo
echo "To run with debugging:"
echo "  make debug"
echo
echo "To write to USB/floppy (DANGEROUS!):"
echo "  sudo dd if=build/tosin_rtos.img of=/dev/sdX bs=512"
echo
echo "Features implemented:"
echo "  ✓ MBR Bootloader (512 bytes)"
echo "  ✓ Preemptive scheduler with priorities"
echo "  ✓ Task management and context switching"
echo "  ✓ Memory management (heap allocator)"
echo "  ✓ Semaphores for synchronization"
echo "  ✓ Message queues for IPC"
echo "  ✓ Interactive shell with commands"
echo "  ✓ Modular kernel design"
echo
