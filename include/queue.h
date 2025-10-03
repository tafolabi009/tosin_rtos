#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"
#include "task.h"
#include "semaphore.h"

/* Message queue structure */
typedef struct {
    void **buffer;              /* Message buffer */
    uint32_t capacity;          /* Queue capacity */
    uint32_t count;             /* Current count */
    uint32_t head;              /* Head index */
    uint32_t tail;              /* Tail index */
    semaphore_t mutex;          /* Mutual exclusion */
    semaphore_t not_empty;      /* Not empty semaphore */
    semaphore_t not_full;       /* Not full semaphore */
    bool_t valid;               /* Queue is valid */
} queue_t;

/* Queue operations */
int32_t queue_create(queue_t **queue, uint32_t capacity);
int32_t queue_send(queue_t *queue, void *msg, uint32_t timeout_ms);
int32_t queue_receive(queue_t *queue, void **msg, uint32_t timeout_ms);
int32_t queue_destroy(queue_t *queue);
uint32_t queue_get_count(queue_t *queue);

#endif /* QUEUE_H */
