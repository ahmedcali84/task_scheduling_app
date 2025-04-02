#include "task.h"

const char *priority_as_cstr[PRIORITY_COUNT] = {
    [LOW] = "LOW",
    [MEDIUM] = "MEDIUM",
    [HIGH] = "HIGH",
};

bool InitQueue(Queue *queue)
{
    queue->count = 0;
    queue->capacity = 0;
    queue->tasks = NULL;
    return true;
}

bool DeallocateQueue(Queue *queue)
{
    if (queue) {
        for (unsigned int i = 0; i < queue->count; ++i) {
            free(queue->tasks[i].message);
        }
        free(queue->tasks);
        queue->capacity = 0;
        queue->count = 0;
        queue->tasks = NULL;
    }
    return true;
}

bool Enqueue(Queue *queue , char *message , Priority priority)
{
    if (!queue) return false;
    if (queue->count == queue->capacity) {
        unsigned int new_capacity = queue->capacity == 0 ? INITIAL_CAP : queue->capacity * 2;
        Task *new_tasks = realloc(queue->tasks, sizeof(*queue->tasks)*new_capacity);
        if (new_tasks == NULL) {
            Log_Out(ERROR, "Failed To Reallocate Queue Items");
            return false;
        }
        queue->tasks = new_tasks;
        queue->capacity = new_capacity;
    }

    char *copy_msg = strdup(message);
    if (copy_msg == NULL) return false;

    Task task = {.message = copy_msg, .id = queue->count, .priority = priority};
    queue->tasks[queue->count++] = task;
    return true;
}

bool RemoveAt(Queue *queue, unsigned int index)
{
    if (queue == NULL) return false;
    if (index >= queue->count) return false;

    free(queue->tasks[index].message);

    // NOTE: Shift Elements
    memmove(&queue->tasks[index], &queue->tasks[index + 1], sizeof(Task) * (queue->count - index - 1));
    queue->count--; // NOTE: decrement count
    return true;
}

bool Dequeue(Queue *queue) { return RemoveAt(queue, 0); }

char *Peek(Queue *queue , unsigned int task_id) { return queue->tasks[task_id].message; }

unsigned int Size(Queue *queue) { return queue->count; }

bool IsEmpty(Queue *queue) { return queue->count == 0; }

void PrintTask(Task task)
{
    Log_Out(INFO, "Task ID: %d | Task: %s | Task Priority: %s.\n", task.id, task.message, priority_as_cstr[task.priority]);
}

void PrintQueue(Queue *queue)
{
    for (unsigned int i = 0; i < queue->count; ++i) {
        PrintTask(queue->tasks[i]);
    }
}

bool task_exists(Queue *queue , char *message)
{
    for (unsigned int i = 0; i < queue->count; ++i) {
        if (strcmp(queue->tasks[i].message, message) == 0) {
            Log_Out(WARN, "Task: %s , Exists.\n", message);
            return true;
        }
    }
    return false;
}