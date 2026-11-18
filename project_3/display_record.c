#include "main.h"

/* Detailed record of one student, including every course. */
void display_student_record(const Student *s) {
    double avg;
    printf("\n------------------------------------------------------------\n");
    printf("  Student ID      : %d\n", s->id);
    printf("  Name            : %s %s\n", s->first_name, s->last_name);
    printf("  Programme       : %s\n", s->program_study);
    printf("  Year of study   : %d\n", s->year_of_study);
    printf("  Courses         : %d\n", s->number_of_courses);
    if (student_average_percent(s, &avg)) printf("  Overall average : %.2f%%\n", avg);
    else                                  printf("  Overall average : N/A\n");

    if (s->number_of_courses > 0) {
        printf("  %-8s %-26s %8s %8s\n", "Code", "Course", "Marks", "Max");
        for (int j = 0; j < s->number_of_courses; j++) {
            const Course *c = &s->courses[j];
            printf("  %-8s %-26.26s %8.2f %8.2f\n",
                   c->course_code, c->course_name, c->marks_obtained, c->maximum_marks);
        }
    }
}

/* Per-course results with pass/fail, plus the overall average. */
void display_academic_performance(const Student *s) {
    double avg;
    printf("\nAcademic performance: %s %s (ID %d, %s)\n",
           s->first_name, s->last_name, s->id, s->program_study);
    if (s->number_of_courses == 0) {
        printf("  No courses registered, so there is no performance data.\n");
        return;
    }
    printf("  %-8s %-26s %8s %8s %8s  %s\n", "Code", "Course", "Marks", "Max", "Percent", "Result");
    for (int j = 0; j < s->number_of_courses; j++) {
        const Course *c = &s->courses[j];
        printf("  %-8s %-26.26s %8.2f %8.2f %7.2f%%  %s\n",
               c->course_code, c->course_name, c->marks_obtained, c->maximum_marks,
               course_percent(c), course_passed(c) ? "PASS" : "FAIL");
    }
    if (student_average_percent(s, &avg))
        printf("  Overall average: %.2f%% -> %s\n", avg, avg >= PASS_PERCENT ? "PASSING" : "FAILING");
}

/* One-line-per-student table. */
void display_student_table(const StudentDB *db) {
    double avg;
    printf("\n%-10s %-14s %-14s %-24s %4s %7s %9s\n",
           "ID", "First name", "Last name", "Programme", "Year", "Courses", "Average");
    printf("---------------------------------------------------------------------------------\n");
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->records[i];
        printf("%-10d %-14.14s %-14.14s %-24.24s %4d %7d ",
               s->id, s->first_name, s->last_name, s->program_study,
               s->year_of_study, s->number_of_courses);
        if (student_average_percent(s, &avg)) printf("%8.2f%%\n", avg);
        else                                  printf("%9s\n", "N/A");
    }
    printf("(%d student%s)\n", db->count, db->count == 1 ? "" : "s");
}

void display_all_students(StudentDB *db) {
    printf("\n--- All registered students ---\n");
    if (db->count == 0) { printf("No students registered yet.\n"); return; }
    display_student_table(db);
}

void view_student(StudentDB *db) {
    int id;
    printf("\n--- View student details ---\n");
    if (db->count == 0) { printf("No students registered yet.\n"); return; }
    if (!read_int("Student ID: ", 1, MAX_STUDENT_ID, &id)) return;
    int idx = search_student_by_id(db, id);
    if (idx < 0) { printf("Error: no student with ID %d.\n", id); return; }
    display_student_record(&db->records[idx]);
}

void display_student_performance(StudentDB *db) {
    int id;
    printf("\n--- Student academic performance ---\n");
    if (db->count == 0) { printf("No students registered yet.\n"); return; }
    if (!read_int("Student ID: ", 1, MAX_STUDENT_ID, &id)) return;
    int idx = search_student_by_id(db, id);
    if (idx < 0) { printf("Error: no student with ID %d.\n", id); return; }
    display_academic_performance(&db->records[idx]);
}
