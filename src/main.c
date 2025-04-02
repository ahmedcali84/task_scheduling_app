#include "task.h"

#define SIXTEEN "             "

void usage(char **argv)
{
    FILE *out = stdout;
    fprintf(out,"USAGE: %s <Task Message> <Priority>.\n",argv[0]);
    fprintf(out,"    Priority: \n      0 -> LOW \n      1 -> MEDIUM \n      2 -> HIGH\n");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        usage(argv);
        return 0;
    }

    const char *file_path = "output.bin";
    
    Queue queue;
    if (!InitQueue(&queue)) return 1;

    // Load existing tasks
    if (!load_file(file_path, &queue)) {
        Log_Out(WARN, "No Existing Tasks Found. Starting Fresh.\n");
    }

    char *buffer = extract_args(argc, argv);
    if (buffer == NULL) return 1;

    if (task_exists(&queue, buffer)) return 1;

    if (!Enqueue(&queue, buffer, atoi(argv[3]))) {
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
