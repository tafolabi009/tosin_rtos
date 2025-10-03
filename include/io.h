#ifndef IO_H
#define IO_H

#include "types.h"

/* Basic I/O functions */
void putchar(char c);
void puts(const char *s);
void printf(const char *fmt, ...);
char getchar(void);
int gets(char *buf, int max_len);

/* String utilities */
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int atoi(const char *str);

#endif /* IO_H */
