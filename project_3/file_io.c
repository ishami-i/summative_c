#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include <errno.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/* File format (plain text, one record per line):                      */
/*   STUDENT_DB_V1                                                     */
/*   <number of students>                                              */
/*   S|id|first|last|programme|year|number_of_courses                   */
/*   C|code|course name|marks|max      (repeated number_of_courses)    */
/* ------------------------------------------------------------------ */

/* Save everything to a temp file first, then rename it over the real file,
 * so a failure in the middle can never destroy the previous save. */
Status write_records_file(const StudentDB *db) {
    if (mkdir(DATA_DIR, 0755) != 0 && errno != EEXIST) {
        perror("Cannot create data directory");
        return ERR_FILE;
    }
    FILE *fp = fopen(DATA_TMP, "w");
    if (!fp) { perror("Cannot open file for writing"); return ERR_FILE; }

    fprintf(fp, "%s\n%d\n", FILE_HEADER, db->count);
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->records[i];
        fprintf(fp, "S|%d|%s|%s|%s|%d|%d\n", s->id, s->first_name, s->last_name,
                s->program_study, s->year_of_study, s->number_of_courses);
        for (int j = 0; j < s->number_of_courses; j++) {
            const Course *c = &s->courses[j];
            fprintf(fp, "C|%s|%s|%.2f|%.2f\n", c->course_code, c->course_name,
                    c->marks_obtained, c->maximum_marks);
        }
    }

    int write_failed = ferror(fp);
    if (fclose(fp) != 0) write_failed = 1;
    if (write_failed) {
        fprintf(stderr, "Error: failed while writing %s.\n", DATA_TMP);
        remove(DATA_TMP);
        return ERR_FILE;
    }
    if (rename(DATA_TMP, DATA_FILE) != 0) {
        perror("Cannot replace data file");
        remove(DATA_TMP);
        return ERR_FILE;
    }
    return OK;
}

/* ---------- loading helpers ---------- */

/* Split `line` in place at FIELD_SEP. Returns the field count, or -1 if more than `max`. */
static int split_fields(char *line, char *fields[], int max) {
    int n = 0;
    fields[n++] = line;
    for (char *p = line; *p; p++) {
        if (*p == FIELD_SEP) {
            if (n == max) return -1;
            *p = '\0';
            fields[n++] = p + 1;
        }
    }
    return n;
}

static int parse_int(const char *s, long min, long max, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (*s == '\0' || *end != '\0' || errno == ERANGE || v < min || v > max) return 0;
    *out = (int)v;
    return 1;
}

static int parse_float(const char *s, float *out) {
    char *end;
    errno = 0;
    double v = strtod(s, &end);
    if (*s == '\0' || *end != '\0' || errno == ERANGE || !(v >= 0.0 && v <= MAX_COURSE_MARKS)) return 0;
    *out = (float)v;
    return 1;
}

static int copy_text(char *dst, size_t size, const char *src) {
    if (!is_valid_text(src, size)) return 0;
    snprintf(dst, size, "%s", src);
    return 1;
}

/* Parse the whole stream into `tmp`. On failure `*msg` and `*lineno` describe the problem. */
static Status parse_stream(FILE *fp, StudentDB *tmp, int *lineno, const char **msg) {
    char line[512], *f[8];
    Student cur;
    int have = 0, declared = 0, expected = 0;
    Status st = ERR_INVALID;
    memset(&cur, 0, sizeof cur);

#define FAIL(code, text) do { st = (code); *msg = (text); goto fail; } while (0)

    if (!fgets(line, sizeof line, fp)) return OK;             /* empty file = empty database */
    *lineno = 1;
    trim_string(line);
    if (strcmp(line, FILE_HEADER) != 0) FAIL(ERR_INVALID, "unrecognised file header");

    if (!fgets(line, sizeof line, fp)) FAIL(ERR_INVALID, "missing student count");
    *lineno = 2;
    trim_string(line);
    if (!parse_int(line, 0, 1000000, &expected)) FAIL(ERR_INVALID, "invalid student count");

    while (fgets(line, sizeof line, fp)) {
        (*lineno)++;
        trim_string(line);
        if (line[0] == '\0') continue;

        int n = split_fields(line, f, 7);
        if (n < 0) FAIL(ERR_INVALID, "too many fields");

        if (strcmp(f[0], "S") == 0) {
            if (n != 7) FAIL(ERR_INVALID, "student record must have 7 fields");
            if (have) {                                       /* finish the previous student */
                if (cur.number_of_courses != declared) FAIL(ERR_INVALID, "course count does not match");
                st = db_add_student(tmp, &cur);
                if (st != OK) FAIL(st, "invalid or duplicate student record");
                have = 0;
                memset(&cur, 0, sizeof cur);
            }
            if (!parse_int(f[1], 1, MAX_STUDENT_ID, &cur.id))      FAIL(ERR_INVALID, "invalid student ID");
            if (!copy_text(cur.first_name, sizeof cur.first_name, f[2]))       FAIL(ERR_INVALID, "invalid first name");
            if (!copy_text(cur.last_name, sizeof cur.last_name, f[3]))         FAIL(ERR_INVALID, "invalid last name");
            if (!copy_text(cur.program_study, sizeof cur.program_study, f[4])) FAIL(ERR_INVALID, "invalid programme");
            if (!parse_int(f[5], MIN_YEAR, MAX_YEAR, &cur.year_of_study))      FAIL(ERR_INVALID, "invalid year of study");
            if (!parse_int(f[6], 0, MAX_COURSES_PER_STUDENT, &declared))       FAIL(ERR_INVALID, "invalid course count");
            have = 1;
        } else if (strcmp(f[0], "C") == 0) {
            Course c;
            memset(&c, 0, sizeof c);
            if (!have || n != 5) FAIL(ERR_INVALID, "course record without a student or with wrong field count");
            if (!is_valid_course_code(f[1]))                        FAIL(ERR_INVALID, "invalid course code");
            snprintf(c.course_code, sizeof c.course_code, "%s", f[1]);
            if (!copy_text(c.course_name, sizeof c.course_name, f[2])) FAIL(ERR_INVALID, "invalid course name");
            if (!parse_float(f[3], &c.marks_obtained) || !parse_float(f[4], &c.maximum_marks))
                FAIL(ERR_INVALID, "invalid marks");
            st = student_add_course(&cur, &c);
            if (st != OK) FAIL(st, "invalid or duplicate course record");
        } else {
            FAIL(ERR_INVALID, "unknown record type");
        }
    }

    if (have) {
        if (cur.number_of_courses != declared) FAIL(ERR_INVALID, "course count does not match");
        st = db_add_student(tmp, &cur);
        if (st != OK) FAIL(st, "invalid or duplicate student record");
        memset(&cur, 0, sizeof cur);
    }
    if (tmp->count != expected) FAIL(ERR_INVALID, "student count does not match the header");
    return OK;

fail:
    student_free(&cur);          /* cur is only non-empty if it was not handed to the database */
    return st;
#undef FAIL
}

/* Load into a temporary database; replace the current one only if everything parsed. */
Status read_records_file(StudentDB *db) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) {
        if (errno == ENOENT) {
            printf("No saved data file (%s) found yet.\n", DATA_FILE);
            return ERR_NOT_FOUND;
        }
        perror("Cannot open data file");
        return ERR_FILE;
    }

    StudentDB tmp;
    if (db_init(&tmp) != OK) {
        fclose(fp);
        printf("Error: %s.\n", status_message(ERR_MEMORY));
        return ERR_MEMORY;
    }

    int lineno = 0;
    const char *msg = "unknown problem";
    Status st = parse_stream(fp, &tmp, &lineno, &msg);
    int read_failed = ferror(fp);
    fclose(fp);

    if (st == OK && read_failed) { st = ERR_FILE; msg = "read error"; }
    if (st != OK) {
        printf("Error: could not load %s (line %d: %s).\n", DATA_FILE, lineno, msg);
        db_free(&tmp);
        return st;
    }

    db_free(db);
    *db = tmp;
    db->modified = 0;
    printf("Loaded %d student record(s) from %s.\n", db->count, DATA_FILE);
    return OK;
}

void save_records(StudentDB *db) {
    printf("\n--- Save records ---\n");
    Status st = write_records_file(db);
    if (st == OK) {
        db->modified = 0;
        printf("Saved %d student record(s) to %s.\n", db->count, DATA_FILE);
    } else {
        printf("Error: records were NOT saved (%s).\n", status_message(st));
    }
}

void load_records(StudentDB *db) {
    printf("\n--- Load records ---\n");
    if (db->modified && !confirm("You have unsaved changes that will be lost. Continue? (y/n): ")) {
        printf("Load cancelled.\n");
        return;
    }
    read_records_file(db);
}
