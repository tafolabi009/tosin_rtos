#include "../include/io.h"
#include "../include/types.h"

/* VGA text mode buffer */
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F  /* White on black */

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

/* Update hardware cursor position */
static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    /* Send low byte to VGA controller */
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(pos & 0xFF)), "d"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0F), "d"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)((pos >> 8) & 0xFF)), "d"((uint16_t)0x3D5));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0E), "d"((uint16_t)0x3D4));
}

/* Scroll screen up by one line */
static void scroll_screen(void) {
    int i;
    
    /* Move all lines up */
    for (i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    
    /* Clear last line */
    for (i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        vga_buffer[i] = (VGA_COLOR << 8) | ' ';
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

/* Put a character to screen */
void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_COLOR << 8) | ' ';
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (VGA_COLOR << 8) | c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }
    
    update_cursor();
}

/* Print string */
void puts(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

/* Simple printf implementation */
static void print_number(uint32_t num, int base) {
    char buffer[32];
    int i = 0;
    const char *digits = "0123456789ABCDEF";
    
    if (num == 0) {
        putchar('0');
        return;
    }
    
    while (num > 0) {
        buffer[i++] = digits[num % base];
        num /= base;
    }
    
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

void printf(const char *fmt, ...) {
    const char *p;
    uint32_t *args = (uint32_t *)((char *)&fmt + sizeof(fmt));
    int arg_index = 0;
    
    for (p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd':
                case 'i':
                    print_number(args[arg_index++], 10);
                    break;
                case 'u':
                    print_number(args[arg_index++], 10);
                    break;
                case 'x':
                    print_number(args[arg_index++], 16);
                    break;
                case 's':
                    puts((const char *)args[arg_index++]);
                    break;
                case 'c':
                    putchar((char)args[arg_index++]);
                    break;
                case '%':
                    putchar('%');
                    break;
                default:
                    putchar(*p);
                    break;
            }
        } else {
            putchar(*p);
        }
    }
}

/* Get character from keyboard */
char getchar(void) {
    uint8_t scancode;
    
    /* Wait for keyboard data */
    while (1) {
        __asm__ volatile("inb %1, %0" : "=a"(scancode) : "d"((uint16_t)0x64));
        if (scancode & 1) {
            break;
        }
    }
    
    __asm__ volatile("inb %1, %0" : "=a"(scancode) : "d"((uint16_t)0x60));
    
    /* Simple scancode to ASCII conversion (US keyboard) */
    static const char scancode_to_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
        0, ' '
    };
    
    if (scancode < sizeof(scancode_to_ascii)) {
        return scancode_to_ascii[scancode];
    }
    
    return 0;
}

/* Get string from keyboard */
int gets(char *buf, int max_len) {
    int i = 0;
    char c;
    
    while (i < max_len - 1) {
        c = getchar();
        
        if (c == '\n') {
            putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                putchar('\b');
            }
        } else if (c != 0) {
            buf[i++] = c;
            putchar(c);
        }
    }
    
    buf[i] = '\0';
    return i;
}

/* String utilities */
int strlen(const char *s) {
    int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && *src) {
        *d++ = *src++;
        n--;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}
