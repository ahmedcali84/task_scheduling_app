#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define INITIAL_CAP 256

typedef struct _Queue {
    char *message;
    unsigned int id;
} Queue;

bool InitQueue(Queue *queue);
bool DeallocateQueue(Queue *queue);
bool Enqueue(Queue *queue ,char *message);
bool Dequeue(Queue *queue);
bool Remove(Queue *queue, unsigned int id);
char *Peek(Queue *queue , unsigned int id);
unsigned int Size(Queue *queue);
bool IsEmpty(Queue *queue);

#endif // QUEUE_H_
