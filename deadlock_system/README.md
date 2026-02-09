# 🔒 Intelligent Deadlock Detection & Recovery System

[![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)](https://en.cppreference.com/)
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)](https://www.linux.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A production-grade implementation of **Banker's Algorithm** for deadlock detection and recovery in operating systems, featuring an interactive ncurses-based TUI, automated logging, and performance profiling.

---

## 🎯 Features

### Core Functionality
- ✅ **Banker's Algorithm Implementation** - Classic deadlock avoidance
- ✅ **Real-time Safety Checking** - Compute safe sequences in nanoseconds
- ✅ **Intelligent Deadlock Recovery** - Automatic victim selection & resource reclamation
- ✅ **Random Scenario Generator** - Configurable deadlock probability testing
- ✅ **Prebuilt Test Cases** - Industry-standard examples (Silberschatz, Galvin)

### Advanced Features
- 📊 **Performance Metrics** - Nanosecond-precision execution time tracking
- 📝 **Comprehensive Logging** - Timestamped session logs with operation history
- 💾 **Persistent Storage** - Save/load scenarios for reproducible testing
- 🎨 **Professional TUI** - Color-coded ncurses interface with keyboard navigation
- ⚡ **Algorithm Complexity Analysis** - Display O(m × n²) runtime characteristics

---

## 🚀 Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install libncurses5-dev libncursesw5-dev

# Fedora/RHEL
sudo dnf install ncurses-devel

# Arch Linux
sudo pacman -S ncurses
```

### Build & Run
```bash
git clone https://github.com/yourusername/deadlock-system.git
cd deadlock-system
make
./deadlock_system
```

---

## 📖 Usage Guide

### Main Menu Navigation
- Use **↑/↓** arrow keys to navigate
- Press **Enter** to select
- Press **ESC** or select "Exit" to quit

### Workflow Example
1. **Generate Random Scenario** → Set parameters (processes, resources, deadlock %)
2. **Check System Safety** → View safe sequence or detect deadlock
3. **Recover from Deadlock** → Automatically terminate victim process
4. **Save Scenario** → Store for future analysis

### Test Cases
- **Case 1**: Classic safe state (5P, 3R) - Silberschatz example
- **Case 2**: Known deadlock (4P, 3R) - Circular wait demonstration
- **Case 3**: Edge case (2P, 2R) - Minimal configuration

---

## 🏗️ Architecture
```
┌─────────────────────────────────────────┐
│         ncurses TUI (ui.c)              │
│  ┌──────────────┐  ┌─────────────────┐  │
│  │ User Input   │  │ Visual Renderer │  │
│  └──────┬───────┘  └────────┬────────┘  │
│         │                   │           │
└─────────┼───────────────────┼───────────┘
          │                   │
┌─────────▼───────────────────▼───────────┐
│      Banker's Algorithm (main.c)        │
│  ┌──────────────────────────────────┐   │
│  │  • calculate_need()              │   │
│  │  • check_safety()                │   │
│  │  • find_deadlock_victim()        │   │
│  │  • recover_deadlock()            │   │
│  └──────────────────────────────────┘   │
└─────────────┬───────────────────────────┘
              │
┌─────────────▼───────────────────────────┐
│       Logging System (logger.c)         │
│  • Timestamped session logs             │
│  • Operation history                    │
│  • Performance metrics                  │
└─────────────────────────────────────────┘
```

---

## 📊 Performance Analysis

### Complexity
- **Time**: O(m × n²) where m = resources, n = processes
- **Space**: O(m × n) for allocation matrices

### Benchmarks (5 processes, 3 resources)
- Safety check: **~800 ns** (0.8 µs)
- Deadlock recovery: **~1200 ns** (1.2 µs)
- Random generation: **~500 ns** (0.5 µs)

---

## 📁 Project Structure
```
deadlock-system/
├── src/
│   ├── common.h          # Shared data structures & function declarations
│   ├── main.c            # Banker's algorithm core implementation
│   ├── ui.c              # ncurses TUI frontend
│   ├── logger.h          # Logging interface
│   └── logger.c          # Logging implementation
├── data/                 # Saved scenario files
├── logs/                 # Session log files
├── obj/                  # Compiled object files
├── Makefile             # Build automation
└── README.md            # Project documentation
```

---

## 🧪 Testing

### Unit Tests
```bash
# Run all prebuilt test cases
./deadlock_system
# Select "Load Test Case" → Choose 1-3
```

### Stress Testing
```bash
# Generate 100 random scenarios with 80% deadlock probability
for i in {1..100}; do
    echo "6\n10\n10\n80\n" | ./deadlock_system
done
```

---

## 🔬 Algorithm Explanation

### Banker's Algorithm Steps
1. **Need Calculation**: `Need[i][j] = Max[i][j] - Allocation[i][j]`
2. **Safety Check**:
```
   Work = Available
   Finish[i] = false for all i
   
   While (∃ i: Finish[i] == false AND Need[i] ≤ Work):
       Work += Allocation[i]
       Finish[i] = true
   
   If all Finish[i] == true → SAFE
   Else → UNSAFE (Deadlock)
```

### Deadlock Recovery
- **Victim Selection**: Process with maximum allocated resources
- **Resource Reclamation**: Release all resources to `Available[]`
- **Re-check**: Run safety algorithm on updated state

---

## 📝 Sample Log Output
```
╔════════════════════════════════════════════════════════╗
║     Deadlock Detection & Recovery System - Log        ║
╚════════════════════════════════════════════════════════╝
Session started: Mon Feb 10 14:23:45 2026

[14:23:46] ℹ [INFO] Loaded test case 2

┌─────────────────────────────────────────────┐
│ Operation: Safety Check                     │
└─────────────────────────────────────────────┘
Processes: 4 | Resources: 3

Allocation Matrix:
  P0      :  3  0  1 
  P1      :  0  2  0 
  P2      :  1  0  3 
  P3      :  1  1  0 

✗ Status: UNSAFE (Deadlock detected)

┌─────── Performance Metrics ───────┐
│ Execution Time:        823 ns     │
│              =      0.823 µs      │
│ Iterations:              4        │
│ Complexity:     O(m × n²)         │
└───────────────────────────────────┘
```

---

## 🎓 Learning Outcomes

This project demonstrates:
- **OS Concepts**: Resource allocation, deadlock detection, process management
- **Data Structures**: 2D arrays, state machines
- **Algorithms**: Greedy approaches, graph traversal (implicit)
- **Systems Programming**: File I/O, logging, performance profiling
- **UI/UX**: Terminal-based interfaces, user input handling

---

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📜 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) for details.

---

## 👨‍💻 Author

**Your Name**  
Computer Science Engineering Student  
[GitHub](https://github.com/yourusername) | [LinkedIn](https://linkedin.com/in/yourprofile) | [Email](mailto:your.email@example.com)

---

## 🙏 Acknowledgments

- **Textbook Reference**: *Operating System Concepts* by Silberschatz, Galvin, Gagne
- **Algorithm**: E.W. Dijkstra's Banker's Algorithm (1965)
- **UI Library**: GNU ncurses

---

## 📚 References

1. Silberschatz, A., Galvin, P. B., & Gagne, G. (2018). *Operating System Concepts* (10th ed.)
2. Dijkstra, E. W. (1965). "Solution of a problem in concurrent programming control"
3. [Linux System Programming](https://man7.org/linux/man-pages/)
4. [ncurses Programming HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)

---

**⭐ If you find this project helpful, please star the repository!**