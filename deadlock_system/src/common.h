#ifndef COMMON_H
#define COMMON_H

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

typedef struct {
    int num_processes;
    int num_resources;
    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int max[MAX_PROCESSES][MAX_RESOURCES];
    int available[MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
    int safe_sequence[MAX_PROCESSES];
    int is_safe;
    char process_names[MAX_PROCESSES][32];
    long long execution_time_ns;  // Performance metric
    int iterations_count;          // Algorithm iterations
} SystemState;

// Core algorithm functions
void calculate_need(SystemState *state);
int check_safety(SystemState *state);
int find_deadlock_victim(SystemState *state);
void recover_deadlock(SystemState *state);

// File operations
void save_scenario(SystemState *state, const char *filename);
int load_scenario(SystemState *state, const char *filename);

// Random generation
void generate_random_scenario(SystemState *state, int num_proc, int num_res, float deadlock_prob);
void generate_test_case(SystemState *state, int case_num);

// Performance metrics
void measure_performance(SystemState *state);
long long get_time_ns();

#endif