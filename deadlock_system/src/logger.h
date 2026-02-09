#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include "common.h"

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS
} LogLevel;

void init_logger();
void log_message(LogLevel level, const char *message);
void log_operation(const char *operation, SystemState *state);
void log_performance(SystemState *state);
void close_logger();

extern FILE *log_file;

#endif