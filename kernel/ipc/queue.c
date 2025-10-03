#include "../include/queue.h"
#include "../include/memory.h"

/* Create a message queue */
int32_t queue_create(queue_t **queue, uint32_t capacity) {
    queue_t *q;
    
    if (!queue || capacity == 0) {
        return ERROR;
    }
    
    q = (queue_t *)kmalloc(sizeof(queue_t));
    if (!q) {
        return ERROR;
    }
    
    q->buffer = (void **)kmalloc(sizeof(void *) * capacity);
    if (!q->buffer) {
        kfree(q);
        return ERROR;
    }
    
    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    
    /* Initialize semaphores */
    if (sem_init(&q->mutex, 1, 1) != SUCCESS) {
        kfree(q->buffer);
        kfree(q);
        return ERROR;
    }
    
    if (sem_init(&q->not_empty, 0, capacity) != SUCCESS) {
        kfree(q->buffer);
        kfree(q);
        return ERROR;
    }
    
    if (sem_init(&q->not_full, capacity, capacity) != SUCCESS) {
        kfree(q->buffer);
        kfree(q);
        return ERROR;
    }
    
    q->valid = TRUE;
    *queue = q;
    
    return SUCCESS;
}

/* Send message to queue */
int32_t queue_send(queue_t *queue, void *msg, uint32_t timeout_ms) {
    if (!queue || !queue->valid) {
        return ERROR;
    }
    
    /* Wait for space in queue */
    if (sem_wait(&queue->not_full, timeout_ms) != SUCCESS) {
        return ERROR;
    }
    
    /* Acquire mutex */
    if (sem_wait(&queue->mutex, timeout_ms) != SUCCESS) {
        sem_post(&queue->not_full);
        return ERROR;
    }
    
    /* Add message to queue */
    queue->buffer[queue->tail] = msg;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    
    /* Release mutex */
    sem_post(&queue->mutex);
    
    /* Signal that queue is not empty */
    sem_post(&queue->not_empty);
    
    return SUCCESS;
}

/* Receive message from queue */
int32_t queue_receive(queue_t *queue, void **msg, uint32_t timeout_ms) {
    if (!queue || !queue->valid || !msg) {
        return ERROR;
    }
    
    /* Wait for message in queue */
    if (sem_wait(&queue->not_empty, timeout_ms) != SUCCESS) {
        return ERROR;
    }
    
    /* Acquire mutex */
    if (sem_wait(&queue->mutex, timeout_ms) != SUCCESS) {
        sem_post(&queue->not_empty);
        return ERROR;
    }
    
    /* Get message from queue */
    *msg = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    
    /* Release mutex */
    sem_post(&queue->mutex);
    
    /* Signal that queue is not full */
    sem_post(&queue->not_full);
    
    return SUCCESS;
}

/* Destroy a queue */
int32_t queue_destroy(queue_t *queue) {
    if (!queue || !queue->valid) {
        return ERROR;
    }
    
    queue->valid = FALSE;
    
    sem_destroy(&queue->mutex);
    sem_destroy(&queue->not_empty);
    sem_destroy(&queue->not_full);
    
    kfree(queue->buffer);
    kfree(queue);
    
    return SUCCESS;
}

/* Get queue count */
uint32_t queue_get_count(queue_t *queue) {
    uint32_t count;
    
    if (!queue || !queue->valid) {
        return 0;
    }
    
    sem_wait(&queue->mutex, 0);
    count = queue->count;
    sem_post(&queue->mutex);
    
    return count;
}
