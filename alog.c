#include "alog.h"

const char * Levels[COUNT_LEVELS] = {
    [INFO]    = "INFO",
    [ERROR]   = "ERROR",
    [DEBUG]   = "DEBUG",
    [WARN] = "WARN",
};

void Logging(FILE *stream, Log_Levels level, const char *fmt, ...)
{
    fprintf(stream, "[%s]: " , Levels[level]);

    va_list args;
    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
}