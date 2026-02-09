#include "logger.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

FILE *log_file = NULL;

void init_logger() {
    mkdir("logs", 0755);
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[256];
    snprintf(filename, sizeof(filename), 
             "logs/session_%04d%02d%02d_%02d%02d%02d.log",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    log_file = fopen(filename, "w");
    if (log_file) {
        fprintf(log_file, "╔════════════════════════════════════════════════════════╗\n");
        fprintf(log_file, "║     Deadlock Detection & Recovery System - Log        ║\n");
        fprintf(log_file, "╚════════════════════════════════════════════════════════╝\n");
        fprintf(log_file, "Session started: %s\n", ctime(&now));
        fflush(log_file);
    }
}

void log_message(LogLevel level, const char *message) {
    if (!log_file) return;
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    const char *level_str[] = {"INFO", "WARN", "ERROR", "SUCCESS"};
    const char *icons[] = {"ℹ", "⚠", "✗", "✓"};
    
    fprintf(log_file, "[%02d:%02d:%02d] %s [%s] %s\n",
            t->tm_hour, t->tm_min, t->tm_sec,
            icons[level], level_str[level], message);
    fflush(log_file);
}

void log_operation(const char *operation, SystemState *state) {
    if (!log_file) return;
    
    fprintf(log_file, "\n┌─────────────────────────────────────────────┐\n");
    fprintf(log_file, "│ Operation: %-32s │\n", operation);
    fprintf(log_file, "└─────────────────────────────────────────────┘\n");
    fprintf(log_file, "Processes: %d | Resources: %d\n", 
            state->num_processes, state->num_resources);
    
    fprintf(log_file, "\nAllocation Matrix:\n");
    for (int i = 0; i < state->num_processes; i++) {
        fprintf(log_file, "  %-8s: ", state->process_names[i]);
        for (int j = 0; j < state->num_resources; j++)
            fprintf(log_file, "%2d ", state->allocation[i][j]);
        fprintf(log_file, "\n");
    }
    
    fprintf(log_file, "\nMax Matrix:\n");
    for (int i = 0; i < state->num_processes; i++) {
        fprintf(log_file, "  %-8s: ", state->process_names[i]);
        for (int j = 0; j < state->num_resources; j++)
            fprintf(log_file, "%2d ", state->max[i][j]);
        fprintf(log_file, "\n");
    }
    
    fprintf(log_file, "\nNeed Matrix:\n");
    for (int i = 0; i < state->num_processes; i++) {
        fprintf(log_file, "  %-8s: ", state->process_names[i]);
        for (int j = 0; j < state->num_resources; j++)
            fprintf(log_file, "%2d ", state->need[i][j]);
        fprintf(log_file, "\n");
    }
    
    fprintf(log_file, "\nAvailable: ");
    for (int j = 0; j < state->num_resources; j++)
        fprintf(log_file, "%d ", state->available[j]);
    fprintf(log_file, "\n");
    
    if (state->is_safe) {
        fprintf(log_file, "\n✓ Status: SAFE\n");
        fprintf(log_file, "  Safe Sequence: ");
        for (int i = 0; i < state->num_processes; i++)
            fprintf(log_file, "%s ", state->process_names[state->safe_sequence[i]]);
        fprintf(log_file, "\n");
    } else {
        fprintf(log_file, "\n✗ Status: UNSAFE (Deadlock detected)\n");
    }
    
    fprintf(log_file, "─────────────────────────────────────────────\n\n");
    fflush(log_file);
}

void log_performance(SystemState *state) {
    if (!log_file) return;
    
    fprintf(log_file, "\n┌─────── Performance Metrics ───────┐\n");
    fprintf(log_file, "│ Execution Time: %10lld ns   │\n", state->execution_time_ns);
    fprintf(log_file, "│              = %10.3f µs   │\n", state->execution_time_ns / 1000.0);
    fprintf(log_file, "│ Iterations:     %10d      │\n", state->iterations_count);
    fprintf(log_file, "│ Complexity:     O(m × n²)       │\n");
    fprintf(log_file, "└───────────────────────────────────┘\n\n");
    fflush(log_file);
}

void close_logger() {
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "\n════════════════════════════════════════════════════════\n");
        fprintf(log_file, "Session ended: %s", ctime(&now));
        fprintf(log_file, "════════════════════════════════════════════════════════\n");
        fclose(log_file);
        log_file = NULL;
    }
}