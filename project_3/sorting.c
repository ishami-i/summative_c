#include "main.h"

/* ---------- comparison functions (negative / 0 / positive like strcmp) ---------- */

int cmp_by_id(const Student *a, const Student *b) {
    return (a->id > b->id) - (a->id < b->id);
}

/* Alphabetical: last name, then first name (case-insensitive). */
int cmp_by_name(const Student *a, const Student *b) {
    int c = str_icmp(a->last_name, b->last_name);
    return c != 0 ? c : str_icmp(a->first_name, b->first_name);
}

/* Students without courses have no average and rank below everyone else. */
int cmp_by_average(const Student *a, const Student *b) {
    double x = -1.0, y = -1.0;
    student_average_percent(a, &x);
    student_average_percent(b, &y);
    return (x > y) - (x < y);
}

int cmp_by_program(const Student *a, const Student *b) {
    int c = str_icmp(a->program_study, b->program_study);
    return c != 0 ? c : cmp_by_id(a, b);
}

/* Manual insertion sort (stable, no qsort). Records are moved as whole structs;
 * each struct only holds a pointer to its course array, so nothing is duplicated. */
void insertion_sort_students(Student *a, int n, StudentCmp cmp, int ascending) {
    for (int i = 1; i < n; i++) {
        Student key = a[i];
        int j = i - 1;
        while (j >= 0 && (ascending ? cmp(&a[j], &key) > 0 : cmp(&a[j], &key) < 0)) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

void sort_students(StudentDB *db) {
    int key, order;
    static const StudentCmp cmps[] = { cmp_by_id, cmp_by_name, cmp_by_average, cmp_by_program };
    static const char *names[] = { "student ID", "name", "overall average", "programme of study" };

    printf("\n--- Sort students ---\n");
    if (db->count < 2) { printf("Need at least two students to sort.\n"); return; }

    printf("  1. Student ID\n  2. Name (alphabetical)\n  3. Overall academic average\n  4. Programme of study\n  0. Back\n");
    if (!read_int("Sort by: ", 0, 4, &key) || key == 0) return;
    if (!read_int("Order (1 = ascending, 2 = descending): ", 1, 2, &order)) return;

    insertion_sort_students(db->records, db->count, cmps[key - 1], order == 1);
    db->modified = 1;
    printf("Students sorted by %s (%s).\n", names[key - 1], order == 1 ? "ascending" : "descending");
    display_student_table(db);
}
