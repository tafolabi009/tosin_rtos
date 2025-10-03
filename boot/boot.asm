; MBR Bootloader for x86
; This bootloader loads the kernel from disk and jumps to it
;
; Memory layout:
; 0x7C00 - Bootloader (512 bytes)
; 0x8000 - Kernel load address

BITS 16
ORG 0x7C00

; Entry point
start:
    ; Setup segments
    cli                     ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00         ; Setup stack below bootloader
    sti                     ; Enable interrupts
    
    ; Print boot message
    mov si, msg_boot
    call print_string
    
    ; Load kernel from disk
    ; Read sectors starting from sector 2 (after MBR)
    mov ah, 0x02           ; BIOS read sector function
    mov al, 32             ; Number of sectors to read (16KB kernel)
    mov ch, 0              ; Cylinder 0
    mov cl, 2              ; Sector 2
    mov dh, 0              ; Head 0
    mov dl, 0x80           ; First hard disk
    mov bx, 0x8000         ; Load to 0x8000
    int 0x13               ; Call BIOS
    
    jc disk_error          ; Jump if error
    
    ; Print success message
    mov si, msg_success
    call print_string
    
    ; Enable A20 line for accessing memory above 1MB
    call enable_a20
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Jump to protected mode code segment
    jmp 0x08:protected_mode

disk_error:
    mov si, msg_error
    call print_string
    jmp halt

; Print string (SI = pointer to null-terminated string)
print_string:
    pusha
.loop:
    lodsb                  ; Load byte from SI to AL
    or al, al              ; Check if zero
    jz .done
    mov ah, 0x0E           ; BIOS teletype function
    mov bh, 0              ; Page 0
    mov bl, 0x07           ; Light gray color
    int 0x10               ; Call BIOS
    jmp .loop
.done:
    popa
    ret

; Enable A20 line
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

BITS 32
protected_mode:
    ; Setup protected mode segments
    mov ax, 0x10           ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000       ; Setup stack at 576KB
    
    ; Jump to kernel entry point
    jmp 0x8000

halt:
    hlt
    jmp halt

; Messages
msg_boot:     db 'Tosin RTOS Booting...', 0x0D, 0x0A, 0
msg_success:  db 'Kernel loaded successfully!', 0x0D, 0x0A, 0
msg_error:    db 'Disk read error!', 0x0D, 0x0A, 0

; Global Descriptor Table
gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment descriptor
    dw 0xFFFF              ; Limit (bits 0-15)
    dw 0x0000              ; Base (bits 0-15)
    db 0x00                ; Base (bits 16-23)
    db 10011010b           ; Access byte (Present, Ring 0, Code, Executable, Readable)
    db 11001111b           ; Flags and limit (bits 16-19)
    db 0x00                ; Base (bits 24-31)

    ; Data segment descriptor
    dw 0xFFFF              ; Limit (bits 0-15)
    dw 0x0000              ; Base (bits 0-15)
    db 0x00                ; Base (bits 16-23)
    db 10010010b           ; Access byte (Present, Ring 0, Data, Writable)
    db 11001111b           ; Flags and limit (bits 16-19)
    db 0x00                ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                 ; Address

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
