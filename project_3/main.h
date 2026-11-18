#ifndef MAIN_H
#define MAIN_H

/*
 * Student Academic Records and Performance Analytics System
 * Shared types, constants and function prototypes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- configuration ---------- */
#define DATA_DIR        "data"
#define DATA_FILE       "data/students.txt"
#define DATA_TMP        "data/students.txt.tmp"
#define FILE_HEADER     "STUDENT_DB_V1"
#define FIELD_SEP       '|'            /* separator used in the data file */

#define NAME_LEN            50
#define PROGRAM_LEN         50
#define CODE_LEN            10
#define COURSE_NAME_LEN     50
#define INITIAL_CAPACITY    4
#define MAX_COURSES_PER_STUDENT 20
#define MIN_YEAR            1
#define MAX_YEAR            7
#define MAX_STUDENT_ID      999999999
#define MAX_COURSE_MARKS    10000.0
#define PASS_PERCENT        50.0       /* a pass is >= 50% of the maximum */

/* ---------- status codes ---------- */
typedef enum {
    OK = 0,
    ERR_MEMORY,
    ERR_DUPLICATE,
    ERR_NOT_FOUND,
    ERR_INVALID,
    ERR_LIMIT,
    ERR_FILE
} Status;

/* ---------- data structures ---------- */

/* One course taken by one student. */
typedef struct {
    char  course_code[CODE_LEN];
    char  course_name[COURSE_NAME_LEN];
    float marks_obtained;
    float maximum_marks;
} Course;

/* One student. `courses` is a heap array (malloc/realloc) owned by the student. */
typedef struct {
    int     id;
    char    first_name[NAME_LEN];
    char    last_name[NAME_LEN];
    char    program_study[PROGRAM_LEN];
    int     year_of_study;
    int     number_of_courses;
    int     course_capacity;
    Course *courses;
} Student;

/* The whole database: a heap array of students that grows/shrinks with realloc. */
typedef struct {
    Student *records;
    int      count;
    int      capacity;
    int      modified;      /* unsaved changes? */
} StudentDB;

/* Menu entry used by the function-pointer menu in main.c */
typedef struct {
    const char *label;
    void (*handler)(StudentDB *db);
} MenuItem;

/* ---------- validation.c ---------- */
void trim_string(char *s);
int  read_line(const char *prompt, char *buf, size_t size);
int  read_int(const char *prompt, int min, int max, int *out);
int  read_float(const char *prompt, double min, double max, float *out);
int  read_text(const char *prompt, char *buf, size_t size);
int  read_course_code(const char *prompt, char *buf, size_t size);
int  confirm(const char *prompt);
int  is_valid_text(const char *s, size_t size);
int  is_valid_course_code(const char *s);
int  str_icmp(const char *a, const char *b);
int  str_icontains(const char *haystack, const char *needle);

/* ---------- student_operations.c ---------- */
const char *status_message(Status s);
Status db_init(StudentDB *db);
void   db_free(StudentDB *db);
void   student_free(Student *s);
Status course_validate(const Course *c);
Status student_validate(const Student *s);
int    student_find_course(const Student *s, const char *code);
Status student_add_course(Student *s, const Course *c);
Status student_update_marks(Student *s, const char *code, float marks);
Status db_add_student(StudentDB *db, const Student *s);   /* takes ownership of s->courses on OK */
Status db_delete_student(StudentDB *db, int id);

void register_student(StudentDB *db);
void update_student(StudentDB *db);
void delete_student(StudentDB *db);
void add_course(StudentDB *db);
void update_marks(StudentDB *db);

/* ---------- display_record.c ---------- */
void display_student_record(const Student *s);
void display_academic_performance(const Student *s);
void display_student_table(const StudentDB *db);
void display_all_students(StudentDB *db);
void view_student(StudentDB *db);
void display_student_performance(StudentDB *db);

/* ---------- searching.c ---------- */
int  search_student_by_id(const StudentDB *db, int id);
int  search_students_by_name(const StudentDB *db, const char *query, int *hits);
int  search_students_by_program(const StudentDB *db, const char *query, int *hits);
void search_students(StudentDB *db);

/* ---------- sorting.c ---------- */
typedef int (*StudentCmp)(const Student *, const Student *);
void insertion_sort_students(Student *a, int n, StudentCmp cmp, int ascending);
int  cmp_by_id(const Student *a, const Student *b);
int  cmp_by_name(const Student *a, const Student *b);
int  cmp_by_average(const Student *a, const Student *b);
int  cmp_by_program(const Student *a, const Student *b);
void sort_students(StudentDB *db);

/* ---------- academic_performance.c ---------- */
double course_percent(const Course *c);
int    course_passed(const Course *c);
int    student_average_percent(const Student *s, double *out);
void   generate_report(StudentDB *db);

/* ---------- file_io.c ---------- */
Status write_records_file(const StudentDB *db);
Status read_records_file(StudentDB *db);
void   save_records(StudentDB *db);
void   load_records(StudentDB *db);

#endif /* MAIN_H */
