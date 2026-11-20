#include "main.h"

// Linear search by ID. Returns the index or -1.
int search_student_by_id(const StudentDB *db, int id) {
    for (int i = 0; i < db->count; i++)
        if (db->records[i].id == id) return i;
    return -1;
}

/* 
Case-insensitive search of first OR last name (partial match allowed).
Fills `hits` (size >= db->count) with indexes; returns how many matched. 
*/
int search_students_by_name(const StudentDB *db, const char *query, int *hits) {
    int n = 0;
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->records[i];
        if (str_icontains(s->first_name, query) || str_icontains(s->last_name, query))
            hits[n++] = i;
    }
    return n;
}

// Case-insensitive search by programme of study (partial match allowed).
int search_students_by_program(const StudentDB *db, const char *query, int *hits) {
    int n = 0;
    for (int i = 0; i < db->count; i++)
        if (str_icontains(db->records[i].program_study, query)) hits[n++] = i;
    return n;
}

void search_students(StudentDB *db) {
    int choice, n = 0;
    char query[NAME_LEN];

    printf("\n--- Search students ---\n");
    if (db->count == 0) { printf("No students registered yet.\n"); return; }

    printf("  1. By student ID\n  2. By first or last name\n  3. By programme of study\n  0. Back\n");
    if (!read_int("Choice: ", 0, 3, &choice) || choice == 0) return;

    int *hits = malloc((size_t)db->count * sizeof *hits);
    if (!hits) { printf("Error: %s.\n", status_message(ERR_MEMORY)); return; }

    if (choice == 1) {
        int id;
        if (!read_int("Student ID: ", 1, MAX_STUDENT_ID, &id)) { free(hits); return; }
        int idx = search_student_by_id(db, id);
        if (idx >= 0) hits[n++] = idx;
    } else if (choice == 2) {
        if (!read_text("Name (or part of it): ", query, sizeof query)) { free(hits); return; }
        n = search_students_by_name(db, query, hits);
    } else {
        if (!read_text("Programme (or part of it): ", query, sizeof query)) { free(hits); return; }
        n = search_students_by_program(db, query, hits);
    }

    if (n == 0) {
        printf("No matching students found.\n");
    } else {
        printf("\n%d matching student(s):\n", n);
        for (int i = 0; i < n; i++) display_student_record(&db->records[hits[i]]);
    }
    free(hits);
}
