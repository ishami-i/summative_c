# Multi-threaded Document Search and Analysis System (C, pthreads)

Searches several text files at the same time, one POSIX thread per file, for a keyword (case-insensitive). Each thread writes its own report.

## How to run
Easiest (asks for the word, uses every `.txt` file in `./data`):

    chmod +x run.sh
    ./run.sh

`run.sh` does `chmod +x ./*`, compiles with `gcc -Wall -Wextra -std=c11 -pthread`, asks for the keyword, then runs the program.

Manual use:

    gcc -Wall -Wextra -std=c11 -pthread -o document_analyzer main.c
    ./document_analyzer "IoT" data/documents_1.txt data/documents_2.txt data/documents_3.txt

## Project layout
    project_5/
    ├── main.c        all source code
    ├── run.sh        compile + run script
    ├── data/         input text files
    └── report/       generated reports (created automatically if missing)

## Output
Terminal: thread creation errors, "Processing ...", "Successfully processed ..." or "ERROR ...", then a summary per file.
Files: `report/<input name without extension>_report.txt`, for example `report/documents_1_report.txt`:

    ===== DOCUMENT ANALYSIS REPORT =====
    Input file          : data/documents_1.txt
    Search keyword      : IoT (case-insensitive)
    Processing status   : SUCCESS
    Total lines         : 4
    Total occurrences   : 5
    Matching lines      : 3
    Matching line no.   : 1, 3, 4

If the keyword is not found, the report says so. If a file cannot be processed, the report is still written with `Processing status : FAILED (reason)`.
The program exits with code 1 if any file failed, otherwise 0.

## Program structure
| Function | Role |
|---|---|
| `init_thread_args` | Fills one `ThreadArgs` (filename, keyword, report path) |
| `process_file` | Thread function: open file, search, write report, print status |
| `search_file` | Reads lines with `getline`, counts lines, occurrences, matching lines |
| `count_occurrences` | Case-insensitive, non-overlapping count in one line |
| `record_match` | Stores a matching line number in a growing array (`realloc`) |
| `generate_report` | Writes the thread's report file |
| `display_results` | Prints the processing summary, returns number of failures |
| `main` | Validates arguments, creates threads, joins them, cleans up |

## How the requirements are met
- **One thread per file:** `main` calls `pthread_create` once per input file.
- **Passing data to threads:** each thread receives a pointer to its own `ThreadArgs` struct (input and results).
- **No synchronization needed:** threads never share data. Each writes only to its own struct and its own report file, and `main` reads the results only after `pthread_join`.
- **Joining:** every successfully created thread is joined before the summary is shown and the program ends.
- **Thread creation failure:** reported on the terminal; the file is marked failed and the other threads continue. Failed threads are not joined.
- **Error handling:** missing/unreadable files, a directory given as a file, read errors, report write errors, empty keyword, missing arguments, and memory allocation failures are all reported without crashing.
- **Case-insensitive search:** characters are compared with `tolower`.

## Notes
- Overlapping matches are not counted twice: searching `IoTIoT` for "IoT" gives 2, but "aaa" searched for "aa" gives 1.
- Reports are named from the input filename, so two inputs with the same name from different folders would overwrite each other's report.
- Output lines from different threads can appear in a different order on each run, because the threads run concurrently.