#include "task.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv);
        return 1;
    }

    const char *file_path = "output.bin";

    Queue queue;
    if (!InitQueue(&queue)) return 1;

    // Load existing tasks
    if (!load_file(file_path, &queue)) {
        Log_Out(WARN, "No Existing Tasks Found. Starting Fresh.\n");
    }

    Task task = parse_args(&queue, argc , argv);

    if (task_exists(&queue, task.message)) return 1;

    if (!Enqueue(&queue, task.message, task.priority)) {
        Log_Out(ERROR, "Failed To Enqueue message: %s.\n", task.message);
        free(task.message);
        return 1;
    }

    // save new tasks
    if (!dump_queue(file_path, &queue)) return 1;
    PrintQueue(&queue);

    free(task.message);
    DeallocateQueue(&queue);
    return 0;
}
