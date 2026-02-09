#define _POSIX_C_SOURCE 199309L

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// Get current time in nanoseconds
long long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void calculate_need(SystemState *state) {
    for (int i = 0; i < state->num_processes; i++) {
        for (int j = 0; j < state->num_resources; j++) {
            state->need[i][j] = state->max[i][j] - state->allocation[i][j];
            if (state->need[i][j] < 0) state->need[i][j] = 0;
        }
    }
}

int check_safety(SystemState *state) {
    long long start = get_time_ns();
    
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int safe_seq[MAX_PROCESSES];
    int count = 0;
    state->iterations_count = 0;

    // Copy available to work
    for (int i = 0; i < state->num_resources; i++)
        work[i] = state->available[i];

    // Find safe sequence using Banker's Algorithm
    while (count < state->num_processes) {
        int found = 0;
        state->iterations_count++;
        
        for (int i = 0; i < state->num_processes; i++) {
            if (!finish[i]) {
                int can_allocate = 1;
                for (int j = 0; j < state->num_resources; j++) {
                    if (state->need[i][j] > work[j]) {
                        can_allocate = 0;
                        break;
                    }
                }
                
                if (can_allocate) {
                    // Simulate allocation
                    for (int j = 0; j < state->num_resources; j++)
                        work[j] += state->allocation[i][j];
                    
                    safe_seq[count++] = i;
                    finish[i] = 1;
                    found = 1;
                }
            }
        }
        
        if (!found) break; // Deadlock detected
    }

    state->is_safe = (count == state->num_processes);
    if (state->is_safe) {
        for (int i = 0; i < state->num_processes; i++)
            state->safe_sequence[i] = safe_seq[i];
    }
    
    state->execution_time_ns = get_time_ns() - start;
    return state->is_safe;
}

int find_deadlock_victim(SystemState *state) {
    // Strategy: kill process with most allocated resources
    int victim = -1;
    int max_resources = -1;
    
    for (int i = 0; i < state->num_processes; i++) {
        int total = 0;
        for (int j = 0; j < state->num_resources; j++)
            total += state->allocation[i][j];
        
        if (total > max_resources) {
            max_resources = total;
            victim = i;
        }
    }
    return victim;
}

void recover_deadlock(SystemState *state) {
    if (state->is_safe) return; // No deadlock
    
    int victim = find_deadlock_victim(state);
    if (victim == -1) return;
    
    // Release victim's resources
    for (int j = 0; j < state->num_resources; j++) {
        state->available[j] += state->allocation[victim][j];
        state->allocation[victim][j] = 0;
        state->max[victim][j] = 0;
    }
    
    // Remove victim from process list (shift)
    for (int i = victim; i < state->num_processes - 1; i++) {
        strcpy(state->process_names[i], state->process_names[i + 1]);
        for (int j = 0; j < state->num_resources; j++) {
            state->allocation[i][j] = state->allocation[i + 1][j];
            state->max[i][j] = state->max[i + 1][j];
        }
    }
    state->num_processes--;
    
    calculate_need(state);
    check_safety(state);
}

void save_scenario(SystemState *state, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    
    fprintf(fp, "%d %d\n", state->num_processes, state->num_resources);
    
    // Save process names
    for (int i = 0; i < state->num_processes; i++)
        fprintf(fp, "%s\n", state->process_names[i]);
    
    // Save allocation matrix
    for (int i = 0; i < state->num_processes; i++) {
        for (int j = 0; j < state->num_resources; j++)
            fprintf(fp, "%d ", state->allocation[i][j]);
        fprintf(fp, "\n");
    }
    
    // Save max matrix
    for (int i = 0; i < state->num_processes; i++) {
        for (int j = 0; j < state->num_resources; j++)
            fprintf(fp, "%d ", state->max[i][j]);
        fprintf(fp, "\n");
    }
    
    // Save available vector
    for (int j = 0; j < state->num_resources; j++)
        fprintf(fp, "%d ", state->available[j]);
    fprintf(fp, "\n");
    
    fclose(fp);
}

int load_scenario(SystemState *state, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    
    if (fscanf(fp, "%d %d", &state->num_processes, &state->num_resources) != 2) {
        fclose(fp);
        return 0;
    }
    
    // Load process names
    for (int i = 0; i < state->num_processes; i++) {
        if (fscanf(fp, "%s", state->process_names[i]) != 1) {
            fclose(fp);
            return 0;
        }
    }
    
    // Load allocation
    for (int i = 0; i < state->num_processes; i++)
        for (int j = 0; j < state->num_resources; j++)
            if (fscanf(fp, "%d", &state->allocation[i][j]) != 1) {
                fclose(fp);
                return 0;
            }
    
    // Load max
    for (int i = 0; i < state->num_processes; i++)
        for (int j = 0; j < state->num_resources; j++)
            if (fscanf(fp, "%d", &state->max[i][j]) != 1) {
                fclose(fp);
                return 0;
            }
    
    // Load available
    for (int j = 0; j < state->num_resources; j++)
        if (fscanf(fp, "%d", &state->available[j]) != 1) {
            fclose(fp);
            return 0;
        }
    
    fclose(fp);
    calculate_need(state);
    return 1;
}

void generate_random_scenario(SystemState *state, int num_proc, int num_res, float deadlock_prob) {
    state->num_processes = num_proc;
    state->num_resources = num_res;
    
    srand(time(NULL));
    
    // Generate process names
    for (int i = 0; i < num_proc; i++)
        snprintf(state->process_names[i], 32, "P%d", i);
    
    // Generate total resources
    int total_available[MAX_RESOURCES];
    for (int j = 0; j < num_res; j++) {
        total_available[j] = 8 + rand() % 12; // 8-19 units
        state->available[j] = total_available[j];
    }
    
    // Generate allocation and max
    for (int i = 0; i < num_proc; i++) {
        for (int j = 0; j < num_res; j++) {
            int max_alloc = total_available[j] / (num_proc / 2 + 1);
            state->allocation[i][j] = rand() % (max_alloc + 1);
            state->max[i][j] = state->allocation[i][j] + rand() % (max_alloc + 1);
            state->available[j] -= state->allocation[i][j];
        }
    }
    
    // Force deadlock with probability
    if ((rand() % 100) < (int)(deadlock_prob * 100)) {
        // Create circular wait
        for (int i = 0; i < num_proc && i < num_res; i++) {
            state->allocation[i][i % num_res] = 2;
            state->max[i][(i + 1) % num_res] = state->allocation[i][(i + 1) % num_res] + 3;
        }
        
        // Deplete resources
        for (int j = 0; j < num_res; j++)
            state->available[j] = rand() % 2;
    }
    
    calculate_need(state);
}

void generate_test_case(SystemState *state, int case_num) {
    switch (case_num) {
        case 1: { // Safe state - Silberschatz example
            state->num_processes = 5;
            state->num_resources = 3;
            
            int alloc[5][3] = {{0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2}};
            int max[5][3] = {{7,5,3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3}};
            int avail[3] = {3,3,2};
            
            for (int i = 0; i < 5; i++) {
                snprintf(state->process_names[i], 32, "P%d", i);
                for (int j = 0; j < 3; j++) {
                    state->allocation[i][j] = alloc[i][j];
                    state->max[i][j] = max[i][j];
                }
            }
            for (int j = 0; j < 3; j++)
                state->available[j] = avail[j];
            break;
        }
        
        case 2: { // Deadlock state
            state->num_processes = 4;
            state->num_resources = 3;
            
            int alloc[4][3] = {{3,0,1}, {0,2,0}, {1,0,3}, {1,1,0}};
            int max[4][3] = {{5,1,1}, {1,3,1}, {3,2,4}, {2,2,2}};
            int avail[3] = {0,0,0};
            
            for (int i = 0; i < 4; i++) {
                snprintf(state->process_names[i], 32, "P%d", i);
                for (int j = 0; j < 3; j++) {
                    state->allocation[i][j] = alloc[i][j];
                    state->max[i][j] = max[i][j];
                }
            }
            for (int j = 0; j < 3; j++)
                state->available[j] = avail[j];
            break;
        }
        
        case 3: { // Minimal edge case
            state->num_processes = 2;
            state->num_resources = 2;
            
            strcpy(state->process_names[0], "Chrome");
            strcpy(state->process_names[1], "Firefox");
            
            state->allocation[0][0] = 1; state->allocation[0][1] = 0;
            state->allocation[1][0] = 0; state->allocation[1][1] = 1;
            
            state->max[0][0] = 2; state->max[0][1] = 1;
            state->max[1][0] = 1; state->max[1][1] = 2;
            
            state->available[0] = 1;
            state->available[1] = 1;
            break;
        }
    }
    
    calculate_need(state);
}