#ifndef TASK_H_
#define TASK_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "alog.h"

#define INITIAL_CAP 256

typedef enum {
    LOW,
    MEDIUM,
    HIGH,
} Priority;

typedef struct _Task {
    char *message;
    unsigned int id;
    Priority priority;
} Task;

typedef struct _Queue {
    Task *tasks;
    unsigned int count;
    unsigned int capacity;
} Queue;

bool InitQueue(Queue *queue);
bool DeallocateQueue(Queue *queue);
bool Enqueue(Queue *queue , char *message);
bool Dequeue(Queue *queue);
bool RemoveAt(Queue *queue, unsigned int index);
char *Peek(Queue *queue , unsigned int task_id);
unsigned int Size(Queue *queue);
bool IsEmpty(Queue *queue);
void PrintQueue(Queue *queue);

char *extract_args(int argc , char **argv);
bool load_file(const char *filepath, Queue *queue);
bool dump_file(const char *filepath , const void *data , unsigned int size);
bool dump_queue(const char *file_path, const Queue *queue);

#endif // TASK_H_
