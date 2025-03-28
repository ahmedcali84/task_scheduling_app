#include "task.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        Log_Out(ERROR, "Usage: %s [MESSAGE].\n", argv[0]);
        return 1;
    }

    const char *file_path = "output.bin";
    
    Queue queue;
    if (!InitQueue(&queue)) return 1;

    // Load existing tasks
    if (!load_file(file_path, &queue)) {
        Log_Out(WARN, "No Existing Tasks Found. Starting Fresh.\n");
    }

    char *buffer = extract_args(argc, argv);
    if (!Enqueue(&queue, buffer)) {
        Log_Out(ERROR, "Failed To Enqueue message: %s.\n", buffer);
        free(buffer);
        return 1;
    }

    free(buffer);

    // save new tasks
    if (!dump_queue(file_path, &queue)) return 1;
    PrintQueue(&queue);

    DeallocateQueue(&queue);
    return 0;
}
