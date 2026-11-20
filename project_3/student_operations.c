#include "main.h"

// Core (non-interactive) operations 

const char *status_message(Status s) {
    switch (s) {
        case OK:            return "OK";
        case ERR_MEMORY:    return "memory allocation failed";
        case ERR_DUPLICATE: return "duplicate record";
        case ERR_NOT_FOUND: return "record not found";
        case ERR_INVALID:   return "invalid data";
        case ERR_LIMIT:     return "limit reached";
        case ERR_FILE:      return "file error";
    }
    return "unknown error";
}

// Allocate the initial student array (calloc zero-fills it).
Status db_init(StudentDB *db) {
    db->count = 0;
    db->modified = 0;
    db->records = calloc(INITIAL_CAPACITY, sizeof(Student));
    if (!db->records) { db->capacity = 0; return ERR_MEMORY; }
    db->capacity = INITIAL_CAPACITY;
    return OK;
}

// Release one student's course array. 
void student_free(Student *s) {
    free(s->courses);
    s->courses = NULL;
    s->number_of_courses = 0;
    s->course_capacity = 0;
}

// Release the whole database (every course array, then the student array). 
void db_free(StudentDB *db) {
    if (!db) return;
    for (int i = 0; i < db->count; i++) student_free(&db->records[i]);
    free(db->records);
    db->records = NULL;
    db->count = db->capacity = 0;
}

// Check one course record.
Status course_validate(const Course *c) {
    if (!is_valid_course_code(c->course_code))              return ERR_INVALID;
    if (!is_valid_text(c->course_name, COURSE_NAME_LEN))    return ERR_INVALID;
    if (!(c->maximum_marks > 0.0f))                         return ERR_INVALID;
    if (!(c->marks_obtained >= 0.0f && c->marks_obtained <= c->maximum_marks))
        return ERR_INVALID;
    return OK;
}

// Check the personal fields of one student 
Status student_validate(const Student *s) {
    if (s->id < 1 || s->id > MAX_STUDENT_ID)                 return ERR_INVALID;
    if (!is_valid_text(s->first_name, NAME_LEN))             return ERR_INVALID;
    if (!is_valid_text(s->last_name, NAME_LEN))              return ERR_INVALID;
    if (!is_valid_text(s->program_study, PROGRAM_LEN))       return ERR_INVALID;
    if (s->year_of_study < MIN_YEAR || s->year_of_study > MAX_YEAR) return ERR_INVALID;
    return OK;
}

// Index of a course inside a student's record, or -1. 
int student_find_course(const Student *s, const char *code) {
    for (int i = 0; i < s->number_of_courses; i++)
        if (str_icmp(s->courses[i].course_code, code) == 0) return i;
    return -1;
}

// Append a validated course; the course array grows with realloc. 
Status student_add_course(Student *s, const Course *c) {
    Status st = course_validate(c);
    if (st != OK) return st;
    if (student_find_course(s, c->course_code) >= 0) return ERR_DUPLICATE;
    if (s->number_of_courses >= MAX_COURSES_PER_STUDENT) return ERR_LIMIT;

    if (s->number_of_courses == s->course_capacity) {
        int new_cap = s->course_capacity ? s->course_capacity * 2 : 4;
        Course *tmp = realloc(s->courses, (size_t)new_cap * sizeof(Course));
        if (!tmp) return ERR_MEMORY;           /* old array is still valid */
        s->courses = tmp;
        s->course_capacity = new_cap;
    }
    s->courses[s->number_of_courses++] = *c;
    return OK;
}

// Change the marks of an existing course. 
Status student_update_marks(Student *s, const char *code, float marks) {
    int idx = student_find_course(s, code);
    if (idx < 0) return ERR_NOT_FOUND;
    if (!(marks >= 0.0f && marks <= s->courses[idx].maximum_marks)) return ERR_INVALID;
    s->courses[idx].marks_obtained = marks;
    return OK;
}

// Add a student to the database (grows with realloc). On OK the database owns s->courses. 
Status db_add_student(StudentDB *db, const Student *s) {
    Status st = student_validate(s);
    if (st != OK) return st;
    if (search_student_by_id(db, s->id) >= 0) return ERR_DUPLICATE;

    if (db->count == db->capacity) {
        int new_cap = db->capacity ? db->capacity * 2 : INITIAL_CAPACITY;
        Student *tmp = realloc(db->records, (size_t)new_cap * sizeof(Student));
        if (!tmp) return ERR_MEMORY;
        db->records = tmp;
        db->capacity = new_cap;
    }
    db->records[db->count++] = *s;
    db->modified = 1;
    return OK;
}

// Remove a student, free its courses, close the gap and shrink the array if it is mostly empty.
Status db_delete_student(StudentDB *db, int id) {
    int idx = search_student_by_id(db, id);
    if (idx < 0) return ERR_NOT_FOUND;

    student_free(&db->records[idx]);
    for (int i = idx; i < db->count - 1; i++) db->records[i] = db->records[i + 1];
    db->count--;
    db->modified = 1;

    if (db->capacity > INITIAL_CAPACITY && db->count <= db->capacity / 4) {
        int new_cap = db->capacity / 2;
        Student *tmp = realloc(db->records, (size_t)new_cap * sizeof(Student));
        if (tmp) { db->records = tmp; db->capacity = new_cap; }   /* failure to shrink is harmless */
    }
    return OK;
}


// Interactive helpers 

// Ask for one course (code not already used by `s`, marks within range). 0 on EOF.
static int read_course(const Student *s, Course *c) {
    char prompt[96];
    memset(c, 0, sizeof *c);

    for (;;) {
        if (!read_course_code("  Course code (e.g. CS101): ", c->course_code, sizeof c->course_code))
            return 0;
        if (s && student_find_course(s, c->course_code) >= 0) {
            printf("  This student already has course %s.\n", c->course_code);
            continue;
        }
        break;
    }
    if (!read_text("  Course name: ", c->course_name, sizeof c->course_name)) return 0;
    if (!read_float("  Maximum marks: ", 0.01, MAX_COURSE_MARKS, &c->maximum_marks)) return 0;

    snprintf(prompt, sizeof prompt, "  Marks obtained (0 - %g): ", c->maximum_marks);
    if (!read_float(prompt, 0.0, c->maximum_marks, &c->marks_obtained)) return 0;
    // keep two decimals so that saving and loading round-trip exactly
    c->marks_obtained = (float)((long)(c->marks_obtained * 100.0 + 0.5)) / 100.0f;
    if (c->marks_obtained > c->maximum_marks) c->marks_obtained = c->maximum_marks;
    return 1;
}

// Prompt showing the current text; empty input keeps it. Returns 0 on EOF. 
static int prompt_keep_text(const char *label, char *field, size_t size) {
    char buf[256], prompt[200];
    for (;;) {
        snprintf(prompt, sizeof prompt, "%s [%s] (Enter = keep): ", label, field);
        if (!read_line(prompt, buf, sizeof buf)) return 0;
        if (buf[0] == '\0') return 1;
        if (!is_valid_text(buf, size)) {
            printf("  Invalid text. Use 1-%zu characters and avoid the '%c' character.\n",
                   size - 1, FIELD_SEP);
            continue;
        }
        memcpy(field, buf, strlen(buf) + 1);   /* length already checked by is_valid_text */
        return 1;
    }
}

// Ask for a student ID and look it up. Returns the index, or -1 (message already printed). 
static int ask_student_index(const StudentDB *db, const char *prompt) {
    int id;
    if (db->count == 0) { printf("No students registered yet.\n"); return -1; }
    if (!read_int(prompt, 1, MAX_STUDENT_ID, &id)) return -1;
    int idx = search_student_by_id(db, id);
    if (idx < 0) printf("Error: no student with ID %d.\n", id);
    return idx;
}

// Menu handlers 

void register_student(StudentDB *db) {
    Student s;
    char prompt[96];
    int id, year, n;
    memset(&s, 0, sizeof s);

    printf("\n--- Register a new student ---\n");
    if (!read_int("Student ID (positive whole number): ", 1, MAX_STUDENT_ID, &id)) return;
    if (search_student_by_id(db, id) >= 0) {
        printf("Error: a student with ID %d already exists.\n", id);
        return;
    }
    s.id = id;
    if (!read_text("First name: ", s.first_name, sizeof s.first_name)) return;
    if (!read_text("Last name: ", s.last_name, sizeof s.last_name)) return;
    if (!read_text("Programme of study: ", s.program_study, sizeof s.program_study)) return;
    snprintf(prompt, sizeof prompt, "Year of study (%d-%d): ", MIN_YEAR, MAX_YEAR);
    if (!read_int(prompt, MIN_YEAR, MAX_YEAR, &year)) return;
    s.year_of_study = year;

    snprintf(prompt, sizeof prompt, "Number of courses to register now (0-%d): ", MAX_COURSES_PER_STUDENT);
    if (!read_int(prompt, 0, MAX_COURSES_PER_STUDENT, &n)) return;

    for (int i = 0; i < n; i++) {
        Course c;
        printf("Course %d of %d\n", i + 1, n);
        if (!read_course(&s, &c)) { student_free(&s); return; }
        Status st = student_add_course(&s, &c);
        if (st != OK) {
            printf("Error: could not add course (%s).\n", status_message(st));
            student_free(&s);
            return;
        }
    }

    Status st = db_add_student(db, &s);
    if (st != OK) {
        printf("Error: could not register student (%s).\n", status_message(st));
        student_free(&s);
        return;
    }
    printf("Student %d (%s %s) registered successfully.\n", id, s.first_name, s.last_name);
}

void update_student(StudentDB *db) {
    printf("\n--- Update student information ---\n");
    int idx = ask_student_index(db, "Student ID to update: ");
    if (idx < 0) return;

    Student *s = &db->records[idx];
    char first[NAME_LEN], last[NAME_LEN], prog[PROGRAM_LEN], buf[64];
    int year = s->year_of_study;
    snprintf(first, sizeof first, "%s", s->first_name);
    snprintf(last, sizeof last, "%s", s->last_name);
    snprintf(prog, sizeof prog, "%s", s->program_study);

    printf("(The student ID cannot be changed.)\n");
    if (!prompt_keep_text("First name", first, sizeof first)) return;
    if (!prompt_keep_text("Last name", last, sizeof last)) return;
    if (!prompt_keep_text("Programme of study", prog, sizeof prog)) return;
    for (;;) {
        char prompt[96], *end;
        snprintf(prompt, sizeof prompt, "Year of study (%d-%d) [%d] (Enter = keep): ",
                 MIN_YEAR, MAX_YEAR, year);
        if (!read_line(prompt, buf, sizeof buf)) return;
        if (buf[0] == '\0') break;
        long v = strtol(buf, &end, 10);
        if (*end != '\0' || v < MIN_YEAR || v > MAX_YEAR) {
            printf("  Invalid input. Enter a whole number between %d and %d.\n", MIN_YEAR, MAX_YEAR);
            continue;
        }
        year = (int)v;
        break;
    }

    snprintf(s->first_name, sizeof s->first_name, "%s", first);
    snprintf(s->last_name, sizeof s->last_name, "%s", last);
    snprintf(s->program_study, sizeof s->program_study, "%s", prog);
    s->year_of_study = year;
    db->modified = 1;
    printf("Student %d updated.\n", s->id);
}

void delete_student(StudentDB *db) {
    printf("\n--- Delete a student ---\n");
    int idx = ask_student_index(db, "Student ID to delete: ");
    if (idx < 0) return;

    Student *s = &db->records[idx];
    int id = s->id;
    printf("About to delete %d: %s %s (%s)\n", id, s->first_name, s->last_name, s->program_study);
    if (!confirm("Are you sure? (y/n): ")) { printf("Deletion cancelled.\n"); return; }

    Status st = db_delete_student(db, id);
    if (st == OK) printf("Student %d deleted.\n", id);
    else          printf("Error: %s.\n", status_message(st));
}

void add_course(StudentDB *db) {
    printf("\n--- Add a course to a student ---\n");
    int idx = ask_student_index(db, "Student ID: ");
    if (idx < 0) return;

    Student *s = &db->records[idx];
    if (s->number_of_courses >= MAX_COURSES_PER_STUDENT) {
        printf("Error: this student already has the maximum of %d courses.\n", MAX_COURSES_PER_STUDENT);
        return;
    }
    Course c;
    if (!read_course(s, &c)) return;
    Status st = student_add_course(s, &c);
    if (st == OK) {
        db->modified = 1;
        printf("Course %s added to student %d.\n", c.course_code, s->id);
    } else {
        printf("Error: could not add course (%s).\n", status_message(st));
    }
}

void update_marks(StudentDB *db) {
    printf("\n--- Update course marks ---\n");
    int idx = ask_student_index(db, "Student ID: ");
    if (idx < 0) return;

    Student *s = &db->records[idx];
    if (s->number_of_courses == 0) { printf("This student has no registered courses.\n"); return; }
    display_academic_performance(s);

    char code[CODE_LEN], prompt[96];
    float marks;
    if (!read_course_code("Course code to update: ", code, sizeof code)) return;
    int ci = student_find_course(s, code);
    if (ci < 0) { printf("Error: student %d has no course %s.\n", s->id, code); return; }

    snprintf(prompt, sizeof prompt, "New marks (0 - %g): ", s->courses[ci].maximum_marks);
    if (!read_float(prompt, 0.0, s->courses[ci].maximum_marks, &marks)) return;
    marks = (float)((long)(marks * 100.0 + 0.5)) / 100.0f;

    Status st = student_update_marks(s, code, marks);
    if (st == OK) {
        db->modified = 1;
        printf("Marks for %s updated to %.2f / %.2f.\n", code, marks, s->courses[ci].maximum_marks);
    } else {
        printf("Error: %s.\n", status_message(st));
    }
}
