#ifndef LOGGING_SYSTEM_H
#define LOGGING_SYSTEM_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    INFO,
    ERROR,
    DEBUG,
    WARN,
    COUNT_LEVELS,
} Log_Levels;

void Logging(FILE *stream, Log_Levels level, const char *fmt, ...);

#define Log_Out(level, fmt, ...) Logging(stdout, level, fmt, ##__VA_ARGS__)
#define Log_File(stream, level, fmt, ...) Logging(stream, level, fmt, ##__VA_ARGS__)

#endif /* LOGGING_SYSTEM_H */
