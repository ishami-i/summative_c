# summative_c

A collection of five coursework projects covering embedded electronics, Linux shell scripting and systems programming in C. Each project lives in its own folder with its own run script and documentation.

| # | Folder | Project | Language / tool | Platform |
|---|---|---|---|---|
| 1 | [`project_1`](project_1) | Sizzling Lappi-Turing – Arduino circuit with buttons, LEDs and a 16x2 LCD | Arduino C++ (`.ino`), Tinkercad | Arduino Uno / Tinkercad |
| 2 | [`project_2`](project_2) | Linux System Health and Resource Monitoring Manager | Bash | Linux only |
| 3 | [`project_3`](project_3) | Student Academic Records and Performance Analytics System | C (C11) | Linux, macOS |
| 4 | [`project_4`](project_4) | Sales Transaction Management System | C (C11) | Linux, macOS |
| 5 | [`project_5`](project_5) | Multi-threaded Document Search and Analysis System | C (C11, POSIX threads) | Linux, macOS |

---

## Repository structure

```
summative_c/
├── README.md
├── .gitignore
├── project_1/                 Arduino + Tinkercad circuit
│   ├── description.md
│   ├── sizzling_lappi_turing_code.ino
│   ├── Sizzling_Lappi-Turing.png
│   ├── schematic_drawing.pdf
│   └── tools&naming.csv
├── project_2/                 Bash system monitor
│   ├── main.sh  common.sh  system_info.sh  cpu_monitor.sh
│   ├── memory_monitor.sh  disk_monitor.sh  process_management.sh
│   ├── health_report.sh  activity_history.sh  clear_activity.sh
│   └── project_description.md
├── project_3/                 Student records system (C)
│   ├── description.md
│   ├── main.c  main.h  student_operations.c  searching.c  sorting.c
│   ├── display_record.c  academic_performance.c  file_io.c  validation.c
│   ├── Makefile  run.sh
│   └── data/students.txt
├── project_4/                 Sales transaction system (C)
│   ├── main.c  main.h  transaction_management.c  transactions_processing.c
│   ├── callback_functions.c  search_sort.c  file_handling.c  utils.c
│   └── run.sh  transactions.txt  Documentation.md
└── project_5/                 Multi-threaded document search (C)
    ├── description.md
    ├── main.c  run.sh
    ├── data/                  input text files
    └── report/                generated reports
```

---

## Requirements

| Tool | Needed for | Check with |
|---|---|---|
| `git` | Cloning the repository | `git --version` |
| `gcc` (C11 support) | Projects 3, 4, 5 | `gcc --version` |
| `make` | Project 3 (optional, `run.sh` also works) | `make --version` |
| `bash` and GNU/Linux tools (`/proc`, `ps`, `free`, `df`) | Project 2 | `bash --version` |
| Arduino IDE, or a free Tinkercad account | Project 1 | – |

On macOS, install the compiler tools with `xcode-select --install`. Project 2 depends on Linux-specific tools, so run it on Linux, WSL, a VM or a Docker container.

---

## Getting started

```bash
git clone https://github.com/ishami-i/summative_c.git
cd summative_c
```

Every run script changes into its own folder first, so you can start them from anywhere. If a script is not executable, run `chmod +x <script>` once.

---

## Project 1 – Sizzling Lappi-Turing (Arduino + LCD)

An Arduino Uno circuit built and simulated in Tinkercad. Two push buttons (entry and exit) are the inputs, a red and a green LED give instant feedback, and a 16x2 LCD in 4-bit mode shows messages.

| Part | Arduino pins |
|---|---|
| LCD RS, E | D12, D11 |
| LCD DB4 – DB7 | D5, D4, D3, D2 |
| Buttons (to ground, `INPUT_PULLUP`) | D6, D7 |
| Red LED / green LED (220 Ω each) | D8 / D9 |

**Run:** open `project_1/sizzling_lappi_turing_code.ino` in the Arduino IDE (board: Arduino Uno) and upload, or paste it into the Code panel of a Tinkercad circuit and start the simulation.
**Details:** [`project_1/description.md`](project_1/description.md) – full pin map, bill of materials and wiring notes.

---

## Project 2 – Linux System Health and Resource Monitoring Manager

An interactive Bash application that monitors CPU, memory, disk, processes and uptime, generates timestamped health reports and keeps an activity log.

**Features:** system information · CPU, memory and disk monitoring with user-defined thresholds · process search and safe termination (protected processes, confirmation, SIGTERM then optional SIGKILL) · health report saved to a chosen directory · activity log you can view and clear.

```bash
cd project_2
chmod +x *.sh
./main.sh
```

**Docs:** [`project_2/project_description.md`](project_2/project_description.md) – menu, functions, commands used, error handling and a test plan.

---

## Project 3 – Student Academic Records and Performance Analytics System

A menu-driven C program that manages students and their course results, with search, sorting, performance analytics and file persistence.

**Highlights:** nested dynamic arrays (`StudentDB` → `Student` → `Course`) using `calloc`/`realloc`/`free` · menu built from an array of function pointers · one sort routine driven by a comparator function pointer · input validation · load/save to `data/students.txt` (a failed load never corrupts the data in memory).

```bash
cd project_3
./run.sh          # builds with gcc and starts the program
# or
make && ./student_system
make debug        # AddressSanitizer + UBSan build
```

**Docs:** [`project_3/description.md`](project_3/description.md) – structure, data structures, memory management, calculations, file format, validation rules and testing.

---

## Project 4 – Sales Transaction Management System

A C program to record and analyse sales transactions, split over several source files.

**Highlights:** function-pointer tables for the menus and the five sales operations (total revenue, average value, highest, lowest, total units) · callback predicates for filtering and counting · comparator callbacks with a hand-written, stable insertion sort · dynamic array that doubles with `realloc` · records stored as `id|product|qty|price|total|date` in `transactions.txt`, loaded at start-up and saved on exit.

```bash
cd project_4
bash run.sh       # chmod +x, compile all .c files with gcc, start sales_app
```

`transactions.txt` already holds 8 sample records.
**Docs:** [`project_4/Documentation.md`](project_4/Documentation.md).

---

## Project 5 – Multi-threaded Document Search and Analysis System

Searches several text files at the same time for a keyword (case-insensitive), one POSIX thread per file. Each thread writes its own report.

```bash
cd project_5
./run.sh                      # asks for the keyword and searches data/document_1..3.txt
# or manually:
gcc -Wall -Wextra -std=c11 -pthread -o document_analyzer main.c
./document_analyzer "IoT" data/document_1.txt data/document_2.txt data/document_3.txt
```

Reports are written to `report/<input name>_report.txt` with the line count, number of occurrences and matching line numbers. The program exits with code 1 if any file failed.
**Docs:** [`project_5/description.md`](project_5/description.md).

---

## Compiler flags used

| Flag | Meaning |
|---|---|
| `-std=c11` | Compile as standard C11 |
| `-Wall -Wextra` | Enable the common warnings |
| `-Wpedantic` | Strict standard conformance (project 3) |
| `-pthread` | Link POSIX threads (project 5) |
| `-fsanitize=address,undefined` | Memory and undefined-behaviour checks (`make debug`, project 3) |

---

## Demo video 

![Demo video](https://img.youtube.com/vi/gJKquugI6Jg/0.jpg)([click to watch](https://www.youtube.com/watch?v=gJKquugI6Jg))

---

## Notes

* Compiled binaries (`student_system`, `sales_app`, `document_analyzer`) are build output. They are rebuilt by each run script and should not be edited.
* Project 2 is the only project that is limited to Linux.
* Output order in project 5 can differ between runs because the threads run concurrently.

---

## Author

ISHAMI Irené ([@ishami-i](https://github.com/ishami-i))