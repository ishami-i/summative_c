# Project 3 – Student Academic Records and Performance Analytics System

A menu-driven program written in C (C11) that manages student records and their course results. It registers and edits students, tracks marks per course, searches and sorts the records, calculates academic performance, produces a summary report, and saves everything to a text file so the data is still there the next time the program starts.

---

## 1. Features

* Register, update and delete students (ID, first name, last name, programme, year of study).
* Add courses to a student and update the marks for a course.
* Display all students in a table, or one student in detail.
* Search by ID, by name (partial, case-insensitive) or by programme.
* Sort by ID, name, overall average or programme, ascending or descending.
* Academic performance: course percentages, per-student average, pass/fail, and a full academic report.
* Save and load records from `data/students.txt`, with an automatic load at start-up and a save prompt on exit.
* Input validation everywhere: the program rejects bad input and asks again instead of crashing.

---

## 2. How to run

From inside the `project_3` folder:

```bash
./run.sh              # creates data/, compiles with gcc, starts the program
```

or with `make`:

```bash
make                  # builds ./student_system
./student_system

make debug            # builds student_system_debug with AddressSanitizer + UBSan
make clean            # removes the binaries
```

If `run.sh` is not executable, run `chmod +x run.sh` once. A C compiler (`gcc`) is the only requirement. The build uses `-std=c11 -Wall -Wextra -Wpedantic -O2`.

The folder ships with `data/students.txt`, which holds 6 sample students (IDs 1001 to 1006). They are loaded automatically when the program starts.

---

## 3. Menu

```
=========== Student Academic Records System ===========
   1. Register a new student
   2. Display all students
   3. View detailed student information
   4. Update student information
   5. Delete a student record
   6. Add a course to a student
   7. Update marks for a course
   8. Display a student's performance
   9. Search students
  10. Sort students
  11. Generate academic report
  12. Save records to file
  13. Load records from file
   0. Exit
-------------------------------------------------------
Students in memory: 6   (unsaved changes)
```

The menu loops until `0` is chosen. If there are unsaved changes, the program asks whether to save before it exits.

---

## 4. Project structure

```
project_3/
├── main.h                   types, constants, prototypes
├── main.c                   start-up load, function-pointer menu, exit prompt
├── student_operations.c     register / update / delete / add course / update marks
├── display_record.c         formatted output
├── searching.c              manual linear searches
├── sorting.c                manual insertion sort and comparators
├── academic_performance.c   averages, pass/fail, report
├── file_io.c                save and load
├── validation.c             safe input and validation helpers
├── Makefile
├── run.sh
├── description.md           this document
└── data/students.txt        saved records
```

| File | Responsibility |
|---|---|
| `main.h` | `Course`, `Student`, `StudentDB`, `MenuItem`, status codes, limits and all prototypes |
| `main.c` | Loads saved data, shows the menu, dispatches to the chosen handler, frees memory on exit |
| `student_operations.c` | Database memory management and every add / update / delete operation |
| `display_record.c` | Student table, detailed view and performance view |
| `searching.c` | Search by ID, name or programme |
| `sorting.c` | `insertion_sort_students` plus four comparator functions |
| `academic_performance.c` | `course_percent`, `course_passed`, `student_average_percent`, `generate_report` |
| `file_io.c` | `write_records_file`, `read_records_file`, `save_records`, `load_records` |
| `validation.c` | `read_int`, `read_float`, `read_text`, `read_course_code`, `confirm`, string helpers |

---

## 5. Data structures

| Structure | Purpose |
|---|---|
| `Course` | `course_code`, `course_name`, `marks_obtained`, `maximum_marks` |
| `Student` | `id`, `first_name`, `last_name`, `program_study`, `year_of_study`, plus `number_of_courses`, `course_capacity` and a heap array `Course *courses` |
| `StudentDB` | `Student *records`, `count`, `capacity`, `modified` (unsaved-changes flag) |
| `MenuItem` | A label and a function pointer `void (*handler)(StudentDB *)` |

The relationship is `StudentDB` 1 ── * `Student` 1 ── * `Course`. Both "many" sides are dynamic arrays, so there is no fixed limit on the number of students. Each student holds at most 20 courses, a limit enforced by validation.

---

## 6. C concepts used

**Pointers and function pointers**
* `StudentDB *db` is passed to every menu handler, so they all work on the same database.
* The menu is an array of `MenuItem`. Adding an operation takes one line in `main.c`.
* `StudentCmp` is a function pointer type, so one sort routine can sort by any key.
* `const` pointers are used for read-only functions such as display and search.

**Dynamic memory**
* `db_init` allocates the first array with `calloc` (4 students).
* `db_add_student` doubles the array with `realloc` when it is full.
* `student_add_course` grows each student's course array with `realloc` (4, 8, 16 …).
* `db_delete_student` frees the student's courses, shifts the records left and shrinks the array when it is 25 % full or less.
* `db_free` frees every course array and then the student array before the program ends.
* `realloc` results are stored in a temporary pointer first, so the old block is never lost if it fails. Every allocation failure is reported through a `Status` code.

**Searching and sorting**
* Linear search by ID (exact) and by name or programme (case-insensitive partial match using a hand-written `str_icontains`).
* Insertion sort written by hand (stable, no `qsort`). Students without courses have no average and rank lowest.

**Multi-file program**
* One header, eight source files, one `Makefile` and one run script.

---

## 7. Academic calculations

| Quantity | Formula |
|---|---|
| Course percentage | marks obtained ÷ maximum marks × 100 |
| Course passed | course percentage ≥ 50 |
| Student overall average | Σ marks obtained ÷ Σ maximum marks × 100 |
| Student passing | overall average ≥ 50 |
| Overall average performance | Σ all marks ÷ Σ all maximum marks × 100 |

The academic report shows the totals, the overall average, the highest and lowest student, the passing and failing counts, the average and pass count per course (grouped by course code) and the number of students per programme. A student with no courses has no average (`N/A`). That student is left out of the highest, lowest, pass and fail figures and is listed separately, so there is no division by zero.

---

## 8. Data file format

`data/students.txt` is plain text, one record per line, fields separated by `|`:

```
STUDENT_DB_V1
<number of students>
S|id|first|last|programme|year|number_of_courses
C|code|course name|marks|max            (repeated for each course)
```

Example:

```
S|1002|Brian|Mugisha|Computer Science|2|3
C|CS101|Programming in C|45.00|100.00
C|CS102|Data Structures|52.00|100.00
```

* **Save** writes `data/students.txt.tmp`, checks for write errors and then renames it over the real file. An interrupted save cannot destroy the previous data.
* **Load** runs at start-up and from menu option 13. A missing file is not an error (fresh start). A malformed file reports the line number and the reason, and the data already in memory is left untouched. Every field is validated again, and duplicate IDs or wrong course counts are rejected.

---

## 9. Validation and error handling

All input is read with `fgets` followed by `strtol` or `strtod` (never `scanf`), so letters, empty input, over-long lines and `nan` / `inf` are rejected and the question is asked again.

| Input | Rule |
|---|---|
| Student ID | Positive whole number up to 999999999, unique |
| First / last name, programme | Required, at most 49 characters, no `|` character |
| Year of study | 1 to 7 |
| Course code | Letters and digits only, at most 9 characters, unique per student |
| Maximum marks | Greater than 0 (up to 10000) |
| Marks obtained | From 0 up to the course maximum |
| Menu choice | Whole number from 0 to 13 |
| Update / delete / view | The ID must exist |
| Files and memory | Open, read, write, rename and allocation failures are all reported |

Other behaviour:
* End of input (Ctrl+D) exits the program cleanly.
* Deleting a student asks for confirmation first.
* Every handler returns a `Status` code (`OK`, `ERR_MEMORY`, `ERR_DUPLICATE`, `ERR_NOT_FOUND`, `ERR_INVALID`, `ERR_LIMIT`, `ERR_FILE`) that is turned into a readable message.

---

## 10. Testing

The tests cover:

* Add, update and delete students and courses.
* Duplicate student ID and duplicate course code rejected.
* Out-of-range marks and invalid menu input rejected.
* Search by ID, name and programme.
* Sort by ID and by average.
* Report generation.
* Save, restart, and automatic load.
* A corrupted data file and a missing data file.
* A stress test with 500 additions and 489 deletions under AddressSanitizer and UBSan (`make debug`), with no errors and no leaks.

Sample output from a session (option 2, then bad input and a duplicate ID):

```
Loaded 6 student record(s) from data/students.txt.

Students in memory: 6
Select an option: 2

--- All registered students ---

ID         First name     Last name      Programme                Year Courses   Average
---------------------------------------------------------------------------------
1001       Alice          Uwase          Computer Science            2       3    75.17%
1002       Brian          Mugisha        Computer Science            2       3    45.17%
1003       Chantal        Ingabire       Electrical Engineering      3       3    83.83%
1004       David          Niyonzima      Electrical Engineering      1       2    50.00%
1005       Esther         Kamanzi        Business Administration     4       2    58.00%
1006       Fabrice        Habimana       Business Administration     1       0       N/A
(6 students)

Select an option: 99
  Invalid input. Enter a whole number between 0 and 13.
Select an option: 1

--- Register a new student ---
Student ID (positive whole number): 1001
Error: a student with ID 1001 already exists.
```