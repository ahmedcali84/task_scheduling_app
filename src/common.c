#include "task.h"

void usage(char **argv)
{
    FILE *out = stdout;
    fprintf(out, "\nTask Manager - Manage priority-based tasks\n\n");
    fprintf(out, "USAGE: %s [OPTIONS]\n\n", argv[0]);
    fprintf(out, "OPTIONS:\n");
    fprintf(out, "  --task <message>    Specify task description (required)\n");
    fprintf(out, "  --priority <value>  Set task priority (required)\n");
    fprintf(out, "                      Values: 0|LOW, 1|MEDIUM, 2|HIGH\n");
    fprintf(out, "  --help              Show this help message\n\n");
    fprintf(out, "EXAMPLES:\n");
    fprintf(out, "  %s --task \"Finish project\" --priority HIGH\n", argv[0]);
    fprintf(out, "  %s --task \"Buy groceries\" --priority 1\n", argv[0]);
    fprintf(out, "  %s --task \"Call mom\" --priority medium --file mytasks.bin\n\n", argv[0]);
    fprintf(out, "NOTES:\n");
    fprintf(out, "  - Tasks are persisted in binary format\n");
    fprintf(out, "  - Priority is case-insensitive\n");
    fprintf(out, "  - Existing tasks are loaded automatically\n");
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

    buffer[k - 1] = '\0'; // NOTE: Null Terminate the strings
    return buffer;
}

Task parse_args(Queue *queue, int argc, char **argv)
{
    char message[BUFFER_LEN] = {0};
    char priority[PRIORITY_LEN] = {0};

    int k = 1;
    while (k < argc) {
        if (strcmp(argv[k], "--help") == 0) {
            usage(argv);
            DeallocateQueue(queue);
            exit(EXIT_SUCCESS);
        } 
        
        else if (strcmp(argv[k], "--task") == 0) {
            k++;
            while (k < argc && strncmp(argv[k], "--", 2) != 0) {
                strcat(message, argv[k]);
                strcat(message, " ");
                k++;
            }
            message[strlen(message) - 1] = '\0';
        }

        else if (strcmp(argv[k], "--priority") == 0) {
            k++;
            if (k < argc) {
                char *priority_arg = argv[k];
                bool valid_priority = false;
                bool is_numeric = true;

                for (char *p = priority_arg; *p != '\0'; ++p) {
                    if (!isdigit(*p)) {
                        is_numeric = false;
                        break;
                    }
                }

                if (is_numeric) {
                    int priority_num = atoi(priority_arg);
                    if (priority_num >= 0 && priority_num < PRIORITY_COUNT) {
                        strncpy(priority, priority_as_cstr[priority_num], PRIORITY_LEN - 1);
                        valid_priority = true;
                    }
                } else {
                    for (int i = 0; i < PRIORITY_COUNT; ++i) {
                        if (strcasecmp(priority_arg, priority_as_cstr[i]) == 0) {
                            strncpy(priority, priority_as_cstr[i], PRIORITY_LEN - 1);
                            valid_priority = true;
                            break;
                        }
                    }
                }

                if (!valid_priority) {
                    Log_Out(WARN, "Invalid Priority: %s. defaulting to LOW.\n", priority_arg);
                    strncpy(priority, priority_as_cstr[LOW], PRIORITY_LEN - 1);
                }
                priority[PRIORITY_LEN - 1] = '\0';
                k++;
            }
        }
        
        else {
            k++;
        }
    }

    Task task = {0};
    task.id = queue->count;
    task.message = strdup(message);

    for (int i = 0; i < PRIORITY_COUNT; ++i) {
        if (strcmp(priority, priority_as_cstr[i]) == 0) {
            task.priority = i;
            break;
        }
    }

    return task;
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
    fclose(f); // NOTE: close file early, buffer holds data now

    if (bytes_read != (size_t) file_size) {
        Log_Out(ERROR, "Failed to Read File: %s (expaected %ld bytes, got %zu bytes).\n", filepath, file_size, bytes_read);
        free(buffer);
        return false;
    }

    // NOTE: extract tasks from buffer
    char *ptr = buffer;
    while (ptr < buffer + file_size) {
        // NOTE: read id
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

        // NOTE: Allocate Memory For Message
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

    // NOTE: Calculate Total Size Needed
    size_t total_size = 0;
    for (unsigned int i = 0; i < queue->count; ++i) {
        total_size += sizeof(uint32_t) * 3; // NOTE: id + msg_len + priority
        total_size += strlen(queue->tasks[i].message); // NOTE: message
    }

    char *buffer = (char *)malloc(total_size);
    if (buffer == NULL) return false;

    char *ptr = buffer;
    for (unsigned int i = 0; i < queue->count; ++i) {
        Task *task = &queue->tasks[i];
        uint32_t id = (uint32_t) task->id;
        uint32_t msg_len = (uint32_t) strlen(task->message);
        uint32_t priority = (uint32_t) task->priority;

        // NOTE: Write Id
        memcpy(ptr, &id, sizeof(id));
        ptr += sizeof(id);

        // NOTE: Write message length
        memcpy(ptr, &msg_len, sizeof(msg_len));
        ptr += sizeof(msg_len);

        // NOTE: Write Priority
        memcpy(ptr, &priority, sizeof(priority));
        ptr += sizeof(priority);

        // NOTE: Write Message Content
        memcpy(ptr, task->message, msg_len);
        ptr += msg_len;
    }

    bool success = dump_file(file_path, buffer, ptr - buffer);
    free(buffer);
    return success;
}
