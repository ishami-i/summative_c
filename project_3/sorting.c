#include "main.h"

// sorting students by id using bubble sort
void sort_students_by_id(student *students, int num_students) {
    for (int i = 0; i < num_students - 1; i++) {
        for (int j = 0; j < num_students - i - 1; j++) {
            if (students[j].id > students[j + 1].id) {
                student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}
// sorting name alphabetically using bubble sort
void sort_students_by_name(student *students, int num_students) {
    for (int i = 0; i < num_students - 1; i++) {
        for (int j = 0; j < num_students - i - 1; j++) {
            if (strcmp(students[j].first_name, students[j + 1].first_name) > 0) {
                student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}
// sorting by program of study using bubble sort
void sort_students_by_program(student *students, int num_students) {
    for (int i = 0; i < num_students - 1; i++) {
        for (int j = 0; j < num_students - i - 1; j++) {
            if (strcmp(students[j].program_study, students[j + 1].program_study) > 0) {
                student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}