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

    // Shift Elements
    memmove(&queue->tasks[index], &queue->tasks[index + 1], sizeof(Task) * (queue->count - index - 1));
    queue->count--; // decrement count
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

char *extract_args(int argc , char **argv)
{
    size_t total_len = 0;
    for (int i = 1; i < argc && i < 3; ++i) {
        total_len += strlen(argv[i]) + 1;
    }

    char *buffer = (char *) malloc(total_len);
    if (buffer == NULL) {
        Log_Out(ERROR, "Failed to Allocate Memory for Buffer to read messages into.\n");
        return NULL;
    }

    int k = 0;
    for (int i = 1; i < argc && i < 3; ++i) {
        size_t len = strlen(argv[i]);
        memcpy(buffer + k, argv[i], len);
        k += len;
        buffer[k++] = ' ';
    }

    buffer[k - 1] = '\0'; // Null Terminate the strings
    return buffer;
}

bool load_file(const char *filepath , Queue *queue)
{
    if (queue == NULL) return false;

    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        if (errno != ENOENT) {
            Log_Out(ERROR, "Failed to Open File: %s for Reading.\n", filepath);
        }
        return false;
    }

    fseek(f, 0 , SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0 , SEEK_SET);

    if (file_size == 0) {
        Log_Out(WARN, "File: %s is Empty.\n", filepath);
        fclose(f);
        return false;
    }

    char *buffer = (char *)malloc(file_size);
    if (buffer == NULL) {
        Log_Out(ERROR, "Failed to Allocate Memory For Buffer.\n");
        fclose(f);
        return false;
    }

    size_t bytes_read = fread(buffer, 1 , file_size , f);
    fclose(f); // close file early, buffer holds data now

    if (bytes_read != (size_t) file_size) {
        Log_Out(ERROR, "Failed to Read File: %s (expaected %ld bytes, got %zu bytes).\n", filepath, file_size, bytes_read);
        free(buffer);
        return false;
    }

    // extract tasks from buffer
    char *ptr = buffer;
    while (ptr < buffer + file_size) {
        // read id
        if (ptr + sizeof(uint32_t) > buffer + file_size) {
            Log_Out(ERROR, "Unexpected End of File while reading Id.\n");
            return false;
        }

        uint32_t id;
        memcpy(&id, ptr, sizeof(id));
        ptr += sizeof(id);

        if (ptr + sizeof(uint32_t) > buffer + file_size) {
            Log_Out(ERROR, "Unexpected End of File while reading message Lenght.\n");
            return false;
        }

        uint32_t msg_len;
        memcpy(&msg_len, ptr, sizeof(msg_len));
        ptr += sizeof(msg_len);

        if (msg_len == 0 || ptr + msg_len > buffer + file_size) {
            Log_Out(ERROR, "Invalid Message Length %u.\n", msg_len);
            return false;
        }

        uint32_t priority;
        memcpy(&priority, ptr, sizeof(priority));
        ptr += sizeof(priority);

        if (ptr + sizeof(uint32_t) > buffer + file_size) {
            Log_Out(ERROR, "Unexpected End of File while reading Priority.\n");
            return false;
        }

        // Allocate Memory For Message
        char *tmp_msg = (char *)malloc(msg_len + 1);
        if (tmp_msg == NULL) {
            Log_Out(ERROR, "Failed To Allocate Memory For tmp message.\n");
            return false;
        }

        memcpy(tmp_msg, ptr, msg_len);
        tmp_msg[msg_len] = '\0';
        ptr += msg_len;

        char *copy_msg = strdup(tmp_msg);
        free(tmp_msg);
        if (copy_msg == NULL) {
            Log_Out(ERROR, "Failed To Copy Message: %s.\n", tmp_msg);
            free(tmp_msg);
            free(buffer);
            return false;
        }

        if (!Enqueue(queue, copy_msg, priority)) {
            Log_Out(ERROR, "Failed to Enqueue Message: %s.\n", tmp_msg);
            free(copy_msg);
            free(buffer);
            return false;
        }

        queue->tasks[queue->count - 1].id = (unsigned int)id;
        free(copy_msg);
    }
    free(buffer);
    return true;
}

bool dump_file(const char *filepath , const void *data , unsigned int size)
{
    FILE *f = fopen(filepath, "wb");
    if (f == NULL) {
        Log_Out(ERROR, "Could Not Open FIle: %s.\n", filepath);
        return false;
    }

    const char *buffer = data;
    while (size > 0) {
        unsigned int chunk = fwrite(buffer, 1, size, f);
        if (ferror(f)) {
            Log_Out(ERROR, "Could Not Write Into File.\n");
            fclose(f);
            return false;
        }

        size   -= chunk;
        buffer += chunk;
    }

    if (f) fclose(f);
    return true;
}

bool dump_queue(const char *file_path, const Queue *queue)
{
    if (queue == NULL || file_path == NULL) return false;

    // Calculate Total Size Needed
    size_t total_size = 0;
    for (unsigned int i = 0; i < queue->count; ++i) {
        total_size += sizeof(uint32_t) * 3; // id + msg_len + priority
        total_size += strlen(queue->tasks[i].message); // message
    }

    char *buffer = (char *)malloc(total_size);
    if (buffer == NULL) return false;

    char *ptr = buffer;
    for (unsigned int i = 0; i < queue->count; ++i) {
        Task *task = &queue->tasks[i];
        uint32_t id = (uint32_t) task->id;
        uint32_t msg_len = (uint32_t) strlen(task->message);
        uint32_t priority = (uint32_t) task->priority;

        // Write Id
        memcpy(ptr, &id, sizeof(id));
        ptr += sizeof(id);

        // Write message length
        memcpy(ptr, &msg_len, sizeof(msg_len));
        ptr += sizeof(msg_len);

        // Write Priority
        memcpy(ptr, &priority, sizeof(priority));
        ptr += sizeof(priority);

        // Write Message Content
        memcpy(ptr, task->message, msg_len);
        ptr += msg_len;
    }

    bool success = dump_file(file_path, buffer, ptr - buffer);
    free(buffer);
    return success;
}