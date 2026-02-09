#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "logger.h"

#define COLOR_SAFE 1
#define COLOR_UNSAFE 2
#define COLOR_MENU 3
#define COLOR_HEADER 4
#define COLOR_INFO 5
#define COLOR_WARNING 6

SystemState system_state = {0};

void init_colors() {
    start_color();
    init_pair(COLOR_SAFE, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_UNSAFE, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_MENU, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_HEADER, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_INFO, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_WARNING, COLOR_YELLOW, COLOR_BLACK);
}

void draw_box_centered(int height, int width, const char *title) {
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    
    WINDOW *win = newwin(height, width, starty, startx);
    box(win, 0, 0);
    wrefresh(win);
    delwin(win);
    
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(starty, startx + (width - strlen(title)) / 2, " %s ", title);
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
}

void draw_banner() {
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(1, (COLS - 60) / 2, "╔════════════════════════════════════════════════════════╗");
    mvprintw(2, (COLS - 60) / 2, "║  Intelligent Deadlock Detection & Recovery System     ║");
    mvprintw(3, (COLS - 60) / 2, "║           Banker's Algorithm Implementation            ║");
    mvprintw(4, (COLS - 60) / 2, "╚════════════════════════════════════════════════════════╝");
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
}

void show_state_screen() {
    clear();
    draw_banner();
    draw_box_centered(LINES - 10, COLS - 10, "CURRENT SYSTEM STATE");
    
    int y = 8;
    int x = 10;
    
    attron(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    mvprintw(y++, x, "Processes: %d | Resources: %d", 
             system_state.num_processes, system_state.num_resources);
    attroff(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    y++;
    
    // Header
    attron(A_BOLD);
    mvprintw(y, x, "%-10s", "Process");
    for (int j = 0; j < system_state.num_resources; j++)
        printw("  A%-2d M%-2d N%-2d", j, j, j);
    attroff(A_BOLD);
    y++;
    
    // Data rows
    for (int i = 0; i < system_state.num_processes; i++) {
        mvprintw(y++, x, "%-10s", system_state.process_names[i]);
        move(y - 1, x + 10);
        for (int j = 0; j < system_state.num_resources; j++) {
            printw("  %2d %2d %2d", 
                   system_state.allocation[i][j],
                   system_state.max[i][j],
                   system_state.need[i][j]);
        }
    }
    
    y += 2;
    attron(COLOR_PAIR(COLOR_INFO));
    mvprintw(y, x, "Available Resources: ");
    for (int j = 0; j < system_state.num_resources; j++)
        printw("R%d=%d ", j, system_state.available[j]);
    attroff(COLOR_PAIR(COLOR_INFO));
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
    
    log_operation("View System State", &system_state);
}

void check_safety_screen() {
    clear();
    draw_banner();
    
    calculate_need(&system_state);
    int safe = check_safety(&system_state);
    
    if (safe) {
        attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        draw_box_centered(16, 70, "✓ SYSTEM IS SAFE");
        attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        
        int y = LINES / 2 - 4;
        int x = (COLS - 60) / 2;
        
        attron(COLOR_PAIR(COLOR_SAFE));
        mvprintw(y, x, "Safe Sequence Found:");
        y++;
        mvprintw(y, x, "  ");
        for (int i = 0; i < system_state.num_processes; i++) {
            printw("%s", system_state.process_names[system_state.safe_sequence[i]]);
            if (i < system_state.num_processes - 1) printw(" → ");
        }
        attroff(COLOR_PAIR(COLOR_SAFE));
        
        y += 3;
        attron(COLOR_PAIR(COLOR_INFO));
        mvprintw(y++, x, "Performance Metrics:");
        mvprintw(y++, x, "  Execution Time: %lld ns (%.3f µs)", 
                 system_state.execution_time_ns,
                 system_state.execution_time_ns / 1000.0);
        mvprintw(y++, x, "  Iterations: %d", system_state.iterations_count);
        mvprintw(y++, x, "  Complexity: O(m × n²) where m=%d, n=%d",
                 system_state.num_resources, system_state.num_processes);
        attroff(COLOR_PAIR(COLOR_INFO));
        
        log_message(LOG_SUCCESS, "System is in safe state");
    } else {
        attron(COLOR_PAIR(COLOR_UNSAFE) | A_BOLD);
        draw_box_centered(14, 70, "✗ DEADLOCK DETECTED!");
        attroff(COLOR_PAIR(COLOR_UNSAFE) | A_BOLD);
        
        int y = LINES / 2 - 3;
        int x = (COLS - 60) / 2;
        
        attron(COLOR_PAIR(COLOR_UNSAFE));
        mvprintw(y++, x, "The system is in an unsafe state.");
        mvprintw(y++, x, "No safe sequence exists for current resource allocation.");
        attroff(COLOR_PAIR(COLOR_UNSAFE));
        
        y++;
        attron(COLOR_PAIR(COLOR_INFO));
        mvprintw(y++, x, "Suggested Actions:");
        mvprintw(y++, x, "  1. Use 'Recover from Deadlock' to terminate a process");
        mvprintw(y++, x, "  2. Release resources manually");
        mvprintw(y++, x, "  3. Load a different scenario");
        attroff(COLOR_PAIR(COLOR_INFO));
        
        log_message(LOG_WARNING, "Deadlock detected - system is unsafe");
    }
    
    log_operation("Safety Check", &system_state);
    log_performance(&system_state);
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void recover_deadlock_screen() {
    clear();
    draw_banner();
    draw_box_centered(18, 70, "DEADLOCK RECOVERY");
    
    int y = LINES / 2 - 6;
    int x = (COLS - 60) / 2;
    
    if (system_state.is_safe) {
        attron(COLOR_PAIR(COLOR_INFO));
        mvprintw(y, x, "System is currently safe. No recovery needed.");
        attroff(COLOR_PAIR(COLOR_INFO));
    } else {
        int victim = find_deadlock_victim(&system_state);
        
        attron(COLOR_PAIR(COLOR_WARNING));
        mvprintw(y++, x, "Selecting victim process for termination...");
        attroff(COLOR_PAIR(COLOR_WARNING));
        
        y++;
        mvprintw(y++, x, "Victim: %s (has most resources)", 
                 system_state.process_names[victim]);
        
        mvprintw(y++, x, "Releasing resources...");
        refresh();
        napms(800);
        
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Terminating victim process: %s",
                 system_state.process_names[victim]);
        log_message(LOG_WARNING, log_msg);
        
        recover_deadlock(&system_state);
        
        y++;
        if (system_state.is_safe) {
            attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
            mvprintw(y++, x, "✓ Recovery Successful!");
            mvprintw(y++, x, "System is now in a safe state.");
            attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
            log_message(LOG_SUCCESS, "Deadlock recovery successful");
        } else {
            attron(COLOR_PAIR(COLOR_UNSAFE));
            mvprintw(y++, x, "✗ Recovery failed. Additional processes may need termination.");
            attroff(COLOR_PAIR(COLOR_UNSAFE));
            log_message(LOG_ERROR, "Deadlock recovery failed");
        }
        
        log_operation("Deadlock Recovery", &system_state);
    }
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void input_system_data() {
    clear();
    draw_banner();
    draw_box_centered(24, 80, "MANUAL DATA ENTRY");
    
    int y = 9;
    int x = (COLS - 70) / 2;
    
    echo();
    curs_set(1);
    
    mvprintw(y, x, "Number of processes (1-%d): ", MAX_PROCESSES);
    scanw("%d", &system_state.num_processes);
    if (system_state.num_processes < 1) system_state.num_processes = 1;
    if (system_state.num_processes > MAX_PROCESSES) 
        system_state.num_processes = MAX_PROCESSES;
    
    mvprintw(y + 2, x, "Number of resources (1-%d): ", MAX_RESOURCES);
    scanw("%d", &system_state.num_resources);
    if (system_state.num_resources < 1) system_state.num_resources = 1;
    if (system_state.num_resources > MAX_RESOURCES) 
        system_state.num_resources = MAX_RESOURCES;
    
    y += 5;
    for (int i = 0; i < system_state.num_processes; i++) {
        mvprintw(y++, x, "Process %d name: ", i);
        scanw("%s", system_state.process_names[i]);
    }
    
    y += 2;
    mvprintw(y++, x, "Enter Allocation Matrix:");
    for (int i = 0; i < system_state.num_processes; i++) {
        mvprintw(y++, x, "%s: ", system_state.process_names[i]);
        for (int j = 0; j < system_state.num_resources; j++)
            scanw("%d", &system_state.allocation[i][j]);
    }
    
    y += 2;
    mvprintw(y++, x, "Enter Max Matrix:");
    for (int i = 0; i < system_state.num_processes; i++) {
        mvprintw(y++, x, "%s: ", system_state.process_names[i]);
        for (int j = 0; j < system_state.num_resources; j++)
            scanw("%d", &system_state.max[i][j]);
    }
    
    y += 2;
    mvprintw(y++, x, "Enter Available resources:");
    for (int j = 0; j < system_state.num_resources; j++)
        scanw("%d", &system_state.available[j]);
    
    calculate_need(&system_state);
    
    noecho();
    curs_set(0);
    
    attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    mvprintw(LINES - 4, (COLS - 30) / 2, "✓ Data saved successfully!");
    attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    
    log_message(LOG_INFO, "Manual system data entered");
    log_operation("Data Entry", &system_state);
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void generate_random_screen() {
    clear();
    draw_banner();
    draw_box_centered(20, 75, "RANDOM SCENARIO GENERATOR");
    
    int y = LINES / 2 - 7;
    int x = (COLS - 65) / 2;
    
    echo();
    curs_set(1);
    
    int num_proc, num_res, deadlock_choice;
    float deadlock_prob;
    
    mvprintw(y, x, "Number of processes (1-%d): ", MAX_PROCESSES);
    scanw("%d", &num_proc);
    if (num_proc < 1) num_proc = 1;
    if (num_proc > MAX_PROCESSES) num_proc = MAX_PROCESSES;
    
    mvprintw(y + 2, x, "Number of resources (1-%d): ", MAX_RESOURCES);
    scanw("%d", &num_res);
    if (num_res < 1) num_res = 1;
    if (num_res > MAX_RESOURCES) num_res = MAX_RESOURCES;
    
    mvprintw(y + 4, x, "Deadlock probability (0-100%%): ");
    scanw("%d", &deadlock_choice);
    deadlock_prob = deadlock_choice / 100.0;
    
    noecho();
    curs_set(0);
    
    attron(COLOR_PAIR(COLOR_INFO));
    mvprintw(y + 7, x, "Generating scenario...");
    attroff(COLOR_PAIR(COLOR_INFO));
    refresh();
    napms(600);
    
    generate_random_scenario(&system_state, num_proc, num_res, deadlock_prob);
    
    attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    mvprintw(y + 9, x, "✓ Scenario generated successfully!");
    attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    
    mvprintw(y + 11, x, "Configuration:");
    mvprintw(y + 12, x, "  Processes:  %d", num_proc);
    mvprintw(y + 13, x, "  Resources:  %d", num_res);
    mvprintw(y + 14, x, "  Deadlock Target: %.0f%%", deadlock_prob * 100);
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), 
             "Generated random scenario: P=%d, R=%d, DeadlockProb=%.0f%%",
             num_proc, num_res, deadlock_prob * 100);
    log_message(LOG_INFO, log_msg);
    log_operation("Random Generation", &system_state);
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void test_case_screen() {
    clear();
    draw_banner();
    draw_box_centered(20, 75, "PREBUILT TEST CASES");
    
    int y = LINES / 2 - 7;
    int x = (COLS - 65) / 2;
    
    const char *cases[] = {
        "1. Classic Safe State (Silberschatz Example)",
        "   → 5 Processes, 3 Resources, Guaranteed Safe",
        "",
        "2. Known Deadlock State",
        "   → 4 Processes, 3 Resources, Circular Wait",
        "",
        "3. Minimal Edge Case",
        "   → 2 Processes, 2 Resources, Simple Test"
    };
    
    for (int i = 0; i < 8; i++) {
        if (i % 3 == 0) attron(A_BOLD);
        mvprintw(y + i, x, "%s", cases[i]);
        if (i % 3 == 0) attroff(A_BOLD);
    }
    
    mvprintw(y + 10, x, "Enter choice (1-3): ");
    echo();
    curs_set(1);
    int choice;
    scanw("%d", &choice);
    noecho();
    curs_set(0);
    
    if (choice >= 1 && choice <= 3) {
        generate_test_case(&system_state, choice);
        attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        mvprintw(y + 12, x, "✓ Test case %d loaded successfully!", choice);
        attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Loaded test case %d", choice);
        log_message(LOG_INFO, log_msg);
        log_operation("Test Case Load", &system_state);
    } else {
        attron(COLOR_PAIR(COLOR_UNSAFE));
        mvprintw(y + 12, x, "✗ Invalid choice");
        attroff(COLOR_PAIR(COLOR_UNSAFE));
    }
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void save_scenario_screen() {
    clear();
    draw_banner();
    draw_box_centered(14, 75, "SAVE SCENARIO");
    
    int y = LINES / 2 - 4;
    int x = (COLS - 65) / 2;
    
    mkdir("data", 0755);
    
    char filename[256];
    mvprintw(y, x, "Enter filename (without extension): ");
    echo();
    curs_set(1);
    scanw("%s", filename);
    noecho();
    curs_set(0);
    
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "data/%s.txt", filename);
    
    save_scenario(&system_state, fullpath);
    
    attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    mvprintw(y + 3, x, "✓ Scenario saved to: %s", fullpath);
    attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Saved scenario to %s", fullpath);
    log_message(LOG_INFO, log_msg);
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void load_scenario_screen() {
    clear();
    draw_banner();
    draw_box_centered(14, 75, "LOAD SCENARIO");
    
    int y = LINES / 2 - 4;
    int x = (COLS - 65) / 2;
    
    char filename[256];
    mvprintw(y, x, "Enter filename (without extension): ");
    echo();
    curs_set(1);
    scanw("%s", filename);
    noecho();
    curs_set(0);
    
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "data/%s.txt", filename);
    
    if (load_scenario(&system_state, fullpath)) {
        attron(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        mvprintw(y + 3, x, "✓ Scenario loaded from: %s", fullpath);
        attroff(COLOR_PAIR(COLOR_SAFE) | A_BOLD);
        
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Loaded scenario from %s", fullpath);
        log_message(LOG_INFO, log_msg);
        log_operation("Scenario Load", &system_state);
    } else {
        attron(COLOR_PAIR(COLOR_UNSAFE));
        mvprintw(y + 3, x, "✗ Failed to load: %s", fullpath);
        attroff(COLOR_PAIR(COLOR_UNSAFE));
        
        log_message(LOG_ERROR, "Failed to load scenario file");
    }
    
    mvprintw(LINES - 2, (COLS - 30) / 2, "Press any key to return...");
    refresh();
    getch();
}

void main_menu() {
    const char *choices[] = {
        "Enter System Data",
        "Show Current State",
        "Check System Safety",
        "Recover from Deadlock",
        "Save Scenario",
        "Load Scenario",
        "Generate Random Scenario",
        "Load Test Case",
        "Exit"
    };
    int n_choices = sizeof(choices) / sizeof(choices[0]);
    int highlight = 0;
    int choice = -1;
    
    while (1) {
        clear();
        draw_banner();
        draw_box_centered(n_choices + 8, 55, "MAIN MENU");
        
        int y = 8;
        int x = (COLS - 45) / 2;
        
        for (int i = 0; i < n_choices; i++) {
            if (i == highlight) {
                attron(A_REVERSE | COLOR_PAIR(COLOR_MENU) | A_BOLD);
                mvprintw(y + i, x, " ► %-40s ", choices[i]);
                attroff(A_REVERSE | COLOR_PAIR(COLOR_MENU) | A_BOLD);
            } else {
                mvprintw(y + i, x, "   %-40s ", choices[i]);
            }
        }
        
        attron(COLOR_PAIR(COLOR_INFO));
        mvprintw(LINES - 3, x, "Use ↑↓ arrows to navigate, Enter to select");
        attroff(COLOR_PAIR(COLOR_INFO));
        
        refresh();
        
        int c = getch();
        switch (c) {
            case KEY_UP:
                highlight = (highlight - 1 + n_choices) % n_choices;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_choices;
                break;
            case 10: // Enter
                choice = highlight;
                break;
        }
        
        if (choice == 0) input_system_data();
        else if (choice == 1) show_state_screen();
        else if (choice == 2) check_safety_screen();
        else if (choice == 3) recover_deadlock_screen();
        else if (choice == 4) save_scenario_screen();
        else if (choice == 5) load_scenario_screen();
        else if (choice == 6) generate_random_screen();
        else if (choice == 7) test_case_screen();
        else if (choice == 8) break;
        
        choice = -1;
    }
}

int main() {
    init_logger();
    log_message(LOG_INFO, "Deadlock Detection System started");
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_colors();
    
    // Initialize with a test case
    generate_test_case(&system_state, 1);
    log_message(LOG_INFO, "Loaded default test case");
    
    main_menu();
    
    endwin();
    
    log_message(LOG_INFO, "System shutdown - normal exit");
    close_logger();
    
    return 0;
}