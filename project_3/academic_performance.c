#include "main.h"

/* Percentage score of one course. */
double course_percent(const Course *c) {
    return c->maximum_marks > 0.0f ? (double)c->marks_obtained / c->maximum_marks * 100.0 : 0.0;
}

/* A course is passed with >= 50% of its maximum marks. */
int course_passed(const Course *c) {
    return (double)c->marks_obtained * 100.0 >= PASS_PERCENT * (double)c->maximum_marks;
}

/* Overall average = total marks obtained / total maximum marks * 100.
 * Returns 0 (and leaves *out untouched) when there are no courses: no division by zero. */
int student_average_percent(const Student *s, double *out) {
    double got = 0.0, max = 0.0;
    for (int i = 0; i < s->number_of_courses; i++) {
        got += s->courses[i].marks_obtained;
        max += s->courses[i].maximum_marks;
    }
    if (s->number_of_courses == 0 || max <= 0.0) return 0;
    *out = got / max * 100.0;
    return 1;
}

typedef struct {
    char   code[CODE_LEN];
    char   name[COURSE_NAME_LEN];
    int    students;
    int    passed;
    double sum_pct;
} CourseStat;

typedef struct {
    char programme[PROGRAM_LEN];
    int  students;
} ProgramStat;

void generate_report(StudentDB *db) {
    printf("\n================ ACADEMIC PERFORMANCE REPORT ================\n");
    if (db->count == 0) { printf("No students registered yet.\n"); return; }

    int total_courses = 0;
    for (int i = 0; i < db->count; i++) total_courses += db->records[i].number_of_courses;

    CourseStat  *cs = calloc((size_t)(total_courses > 0 ? total_courses : 1), sizeof *cs);
    ProgramStat *ps = calloc((size_t)(unsigned)db->count, sizeof *ps);
    if (!cs || !ps) {
        printf("Error: %s.\n", status_message(ERR_MEMORY));
        free(cs); free(ps);
        return;
    }
    int ncs = 0, nps = 0;

    double all_got = 0.0, all_max = 0.0;
    int best = -1, worst = -1, passing = 0, failing = 0, no_courses = 0;
    double best_avg = 0.0, worst_avg = 0.0;

    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->records[i];
        double avg;

        /* students per programme */
        int p = 0;
        while (p < nps && str_icmp(ps[p].programme, s->program_study) != 0) p++;
        if (p == nps) { snprintf(ps[nps].programme, sizeof ps[nps].programme, "%s", s->program_study); nps++; }
        ps[p].students++;

        /* per-student statistics */
        if (student_average_percent(s, &avg)) {
            if (best < 0 || avg > best_avg)   { best = i;  best_avg = avg; }
            if (worst < 0 || avg < worst_avg) { worst = i; worst_avg = avg; }
            if (avg >= PASS_PERCENT) passing++; else failing++;
        } else {
            no_courses++;
        }

        /* per-course statistics */
        for (int j = 0; j < s->number_of_courses; j++) {
            const Course *c = &s->courses[j];
            all_got += c->marks_obtained;
            all_max += c->maximum_marks;
            int k = 0;
            while (k < ncs && str_icmp(cs[k].code, c->course_code) != 0) k++;
            if (k == ncs) {
                snprintf(cs[ncs].code, sizeof cs[ncs].code, "%s", c->course_code);
                snprintf(cs[ncs].name, sizeof cs[ncs].name, "%s", c->course_name);
                ncs++;
            }
            cs[k].students++;
            cs[k].sum_pct += course_percent(c);
            if (course_passed(c)) cs[k].passed++;
        }
    }

    printf("Total registered students            : %d\n", db->count);
    printf("Total courses registered (all)       : %d\n", total_courses);
    if (all_max > 0.0) printf("Overall average performance          : %.2f%%\n", all_got / all_max * 100.0);
    else               printf("Overall average performance          : N/A (no marks recorded)\n");

    if (best >= 0) {
        printf("Highest average                      : %.2f%%  (%s %s, ID %d)\n", best_avg,
               db->records[best].first_name, db->records[best].last_name, db->records[best].id);
        printf("Lowest average                       : %.2f%%  (%s %s, ID %d)\n", worst_avg,
               db->records[worst].first_name, db->records[worst].last_name, db->records[worst].id);
    } else {
        printf("Highest / lowest average             : N/A (no student has courses)\n");
    }
    printf("Students passing (average >= %.0f%%)   : %d\n", PASS_PERCENT, passing);
    printf("Students failing (average <  %.0f%%)   : %d\n", PASS_PERCENT, failing);
    if (no_courses > 0)
        printf("Students with no courses (excluded)  : %d\n", no_courses);

    printf("\nAverage performance per course:\n");
    if (ncs == 0) {
        printf("  (no courses registered)\n");
    } else {
        printf("  %-8s %-26s %9s %9s %12s\n", "Code", "Course", "Students", "Average", "Passed");
        for (int k = 0; k < ncs; k++)
            printf("  %-8s %-26.26s %9d %8.2f%% %6d/%-5d\n", cs[k].code, cs[k].name,
                   cs[k].students, cs[k].sum_pct / cs[k].students, cs[k].passed, cs[k].students);
    }

    printf("\nStudents per programme of study:\n");
    for (int p = 0; p < nps; p++) printf("  %-30s %d\n", ps[p].programme, ps[p].students);
    printf("==============================================================\n");

    free(cs);
    free(ps);
}
