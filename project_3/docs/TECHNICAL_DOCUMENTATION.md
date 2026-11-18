# Student Academic Records and Performance Analytics System – Technical Documentation

## 1. Structure definitions and relationships (`main.h`)

| Structure   | Purpose |
|-------------|---------|
| `Course`    | One course result: `course_code`, `course_name`, `marks_obtained`, `maximum_marks`. |
| `Student`   | Personal data (`id`, names, `program_study`, `year_of_study`) plus `number_of_courses`, `course_capacity` and `Course *courses` (a heap array owned by the student). |
| `StudentDB` | `Student *records`, `count`, `capacity`, `modified` – the whole database. |
| `MenuItem`  | Label + function pointer, used for the menu in `main.c`. |

Relationship: `StudentDB` 1 ── * `Student` 1 ── * `Course`. Both "many" sides are dynamic arrays, so
the program has no fixed limit on students and no fixed array of courses inside each student
(a sanity cap of 20 courses per student is enforced by validation only).

## 2. Pointer usage

* `StudentDB *db` is passed to every menu handler, so handlers modify the one shared database.
* `Student *s = &db->records[idx]` gives direct access to a record; `s->courses[j]` accesses courses.
* `const Student *` / `const StudentDB *` is used for read-only functions (display, search, averages).
* `int *hits` (search) is a heap array of matching indexes. `StudentCmp` is a function pointer
  `int (*)(const Student*, const Student*)` that lets one sort routine sort by any key.
* The menu is an array of `MenuItem` containing function pointers, so adding an operation is one line.

## 3. Dynamic memory allocation and deallocation

| Where | What |
|-------|------|
| `db_init` | `calloc` the initial array of `INITIAL_CAPACITY` students. |
| `db_add_student` | when full, `realloc` to double the capacity. |
| `student_add_course` | `realloc` the student's course array (4, 8, 16 …). |
| `db_delete_student` | frees the student's course array, shifts records left, `realloc` to half the size when ≤ 25 % full. |
| `search_students`, `generate_report` | temporary `malloc`/`calloc` buffers, freed before returning. |
| `db_free` / `student_free` | free every course array, then the student array. Called before exit. |

Failure handling: every allocation is checked. `realloc` results go into a temporary pointer first,
so the old block is never lost on failure; functions return `ERR_MEMORY` and the menu reports it.
Ownership rule: `db_add_student` takes ownership of `s->courses` only when it returns `OK`;
otherwise the caller frees it. `read_records_file` builds a temporary database and swaps it in only when
the whole file parsed, so a failed load cannot leak or corrupt the current data.

## 4. Searching and sorting algorithms

**Searching** (`searching.c`) – manual linear search over the array.
* By ID: first exact match, O(n) (IDs are unique).
* By first/last name and by programme: case-insensitive partial match using a hand-written
  `str_icontains`; every match is returned and displayed.

**Sorting** (`sorting.c`) – manual **insertion sort** (stable, O(n²) worst case, no `qsort`).
Keys: ID, name (last then first, case-insensitive), overall average, programme. Ascending or descending.
Students without courses have no average and rank lowest.

## 5. File handling (`file_io.c`)

Plain text, one record per line, `|` separated:

```
STUDENT_DB_V1
<number of students>
S|id|first|last|programme|year|number_of_courses
C|code|course name|marks|max            (repeated per course)
```

* **Save** writes `data/students.txt.tmp`, checks for write errors, then `rename()`s it over the real file,
  so an interrupted save cannot destroy the previous data.
* **Load** runs at start-up and from the menu. A missing file is not an error (fresh start).
  A malformed file reports the line number and reason, and the in-memory data is left untouched.
  Every field is re-validated, duplicate IDs and wrong course counts are rejected.
* The program asks whether to save when exiting with unsaved changes.

## 6. Academic performance calculations (`academic_performance.c`)

* Course percentage = marks / maximum × 100. A course is **passed** when marks ≥ 50 % of maximum.
* **Student overall average** = Σ marks obtained ÷ Σ maximum marks × 100 (uses each course's maximum, as required).
  A student is **passing** when this average is ≥ 50 %.
* **Overall average performance** = Σ all marks ÷ Σ all maximum marks × 100.
* Report: totals, overall average, highest/lowest student, passing/failing counts, average and pass count
  per course (grouped by course code), students per programme (case-insensitive).
* Division by zero is prevented: a student with no courses has no average (`N/A`) and is excluded from
  highest/lowest/pass/fail, and is listed separately.

## 7. Error handling and testing

Input is read with `fgets` + `strtol`/`strtod` (never `scanf`), so letters, empty input, over-long lines and
`nan`/`inf` are rejected and re-prompted. Validated: ID range and uniqueness, names (length, no `|`),
year 1–7, course code (letters/digits, unique per student), maximum marks > 0, 0 ≤ marks ≤ maximum,
menu choices, update/delete of non-existent IDs, file and memory failures.

Testing performed (see `sample_run.txt`): CRUD on students and courses, duplicate ID and duplicate course
rejection, out-of-range marks, invalid menu input, search by ID/name/programme, sort by ID and average,
report generation, save → restart → automatic load, corrupted file, missing file, and a stress test
(500 adds + 489 deletes) under AddressSanitizer/UBSan with no errors or leaks.

Build: `make` or `./run.sh` (`make debug` builds with sanitizers).

## 8. File overview

| File | Responsibility |
|------|----------------|
| `main.h` | types, constants, prototypes |
| `main.c` | startup load, function-pointer menu, exit/save prompt |
| `student_operations.c` | register / update / delete / add course / update marks, database memory management |
| `display_record.c` | formatted output |
| `searching.c`, `sorting.c` | manual search and sort |
| `academic_performance.c` | averages and the report |
| `file_io.c` | save and load |
| `validation.c` | safe input and validation helpers |
