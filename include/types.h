#ifndef TYPES_H
#define TYPES_H

/* Basic type definitions for RTOS */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

typedef uint32_t           size_t;
typedef int32_t            ssize_t;

#define NULL ((void*)0)

#define TRUE  1
#define FALSE 0

typedef uint8_t bool_t;

/* Return codes */
#define SUCCESS  0
#define ERROR   -1

#endif /* TYPES_H */
