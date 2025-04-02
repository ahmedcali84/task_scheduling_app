#ifndef TASK_H_
#define TASK_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "../thirdparty/alog.h"

#define INITIAL_CAP  10
#define BUFFER_LEN   256
#define PRIORITY_LEN 10

typedef enum {
    LOW,
    MEDIUM,
    HIGH,
    PRIORITY_COUNT,
} Priority;

extern const char *priority_as_cstr[PRIORITY_COUNT];

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
bool Enqueue(Queue *queue , char *message, Priority priority);
bool Dequeue(Queue *queue);
bool RemoveAt(Queue *queue, unsigned int index);
char *Peek(Queue *queue , unsigned int task_id);
unsigned int Size(Queue *queue);
bool IsEmpty(Queue *queue);

void PrintTask(Task task);
void PrintQueue(Queue *queue);
bool task_exists(Queue *queue , char *message);

void usage(char **argv);
char *extract_args(int argc , char **argv);
Task parse_args(Queue *queue, int argc, char **argv);
bool load_file(const char *filepath, Queue *queue);
bool dump_file(const char *filepath , const void *data , unsigned int size);
bool dump_queue(const char *file_path, const Queue *queue);

#endif // TASK_H_
