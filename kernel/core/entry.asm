; Kernel entry point and context switching for x86
; This file contains the initial kernel entry and context switch assembly

BITS 32
EXTERN kmain
EXTERN scheduler_tick_handler

GLOBAL _start
GLOBAL context_switch
GLOBAL enable_interrupts
GLOBAL disable_interrupts

section .text

; Kernel entry point
_start:
    ; Clear the screen
    mov edi, 0xB8000       ; VGA text buffer
    mov ecx, 2000          ; 80x25 characters
    mov ax, 0x0F20         ; White on black space
    rep stosw
    
    ; Call kernel main
    call kmain
    
    ; Halt if kernel returns
    cli
.hang:
    hlt
    jmp .hang

; Context switch function
; void context_switch(cpu_context_t *old_ctx, cpu_context_t *new_ctx)
context_switch:
    ; Save old context
    mov eax, [esp + 4]     ; old_ctx pointer
    test eax, eax
    jz .restore            ; If NULL, skip save
    
    ; Save general purpose registers
    mov [eax + 0], ebx     ; Save EBX (EAX saved by caller)
    mov [eax + 4], ecx
    mov [eax + 8], edx
    mov [eax + 12], esi
    mov [eax + 16], edi
    mov [eax + 20], ebp
    
    ; Save ESP (before call)
    lea ecx, [esp + 4]
    mov [eax + 24], ecx
    
    ; Save return address as EIP
    mov ecx, [esp]
    mov [eax + 28], ecx
    
    ; Save EFLAGS
    pushfd
    pop ecx
    mov [eax + 32], ecx
    
    ; Save segment registers
    mov cx, cs
    mov [eax + 36], cx
    mov cx, ss
    mov [eax + 40], cx
    mov cx, ds
    mov [eax + 44], cx
    mov cx, es
    mov [eax + 48], cx
    mov cx, fs
    mov [eax + 52], cx
    mov cx, gs
    mov [eax + 56], cx

.restore:
    ; Restore new context
    mov eax, [esp + 8]     ; new_ctx pointer
    
    ; Restore segment registers
    mov cx, [eax + 44]
    mov ds, cx
    mov cx, [eax + 48]
    mov es, cx
    mov cx, [eax + 52]
    mov fs, cx
    mov cx, [eax + 56]
    mov gs, cx
    
    ; Restore general purpose registers
    mov ebx, [eax + 0]
    mov ecx, [eax + 4]
    mov edx, [eax + 8]
    mov esi, [eax + 12]
    mov edi, [eax + 16]
    mov ebp, [eax + 20]
    mov esp, [eax + 24]
    
    ; Restore EFLAGS
    push dword [eax + 32]
    popfd
    
    ; Jump to new task
    jmp [eax + 28]

; Enable interrupts
enable_interrupts:
    sti
    ret

; Disable interrupts
disable_interrupts:
    cli
    ret

; IDT setup
section .data
align 4
idt_entries:
    times 256 dd 0, 0      ; 256 IDT entries

idt_ptr:
    dw 256*8 - 1           ; IDT limit
    dd idt_entries         ; IDT base

section .text
GLOBAL setup_idt
setup_idt:
    ; Setup timer interrupt (IRQ0)
    mov eax, timer_interrupt_handler
    mov [idt_entries + 32*8], ax
    shr eax, 16
    mov [idt_entries + 32*8 + 6], ax
    mov word [idt_entries + 32*8 + 2], 0x08  ; Code segment
    mov byte [idt_entries + 32*8 + 5], 0x8E  ; Present, Ring 0, Interrupt gate
    
    ; Load IDT
    lidt [idt_ptr]
    
    ; Program PIC (Programmable Interrupt Controller)
    ; Remap IRQs to 0x20-0x2F
    mov al, 0x11
    out 0x20, al           ; Master PIC
    out 0xA0, al           ; Slave PIC
    
    mov al, 0x20
    out 0x21, al           ; Master PIC offset
    mov al, 0x28
    out 0xA1, al           ; Slave PIC offset
    
    mov al, 0x04
    out 0x21, al           ; Tell master about slave
    mov al, 0x02
    out 0xA1, al           ; Tell slave its cascade
    
    mov al, 0x01
    out 0x21, al           ; 8086 mode
    out 0xA1, al
    
    ; Unmask timer interrupt (IRQ0)
    in al, 0x21
    and al, 0xFE
    out 0x21, al
    
    ; Program PIT (Programmable Interval Timer) for 100Hz
    mov al, 0x36
    out 0x43, al
    mov ax, 11932          ; 1193182 / 100
    out 0x40, al
    mov al, ah
    out 0x40, al
    
    ret

; Timer interrupt handler
timer_interrupt_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Setup kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    
    ; Call C handler
    call scheduler_tick_handler
    
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

section .bss
