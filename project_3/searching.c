#include "main.h"

// searching with id
int search_student_by_id(student *students, int num_students, int id) {
    for (int i = 0; i < num_students; i++) {
        if (students[i].id == id) {
            return i; // return index of the student
        }
    }
    return -1; // student not found
}

// searching with name
int search_student_by_name(student *students, int num_students, const char *first_name, const char *last_name) {
    for (int i = 0; i < num_students; i++) {
        if (strcmp(students[i].first_name, first_name) == 0 && strcmp(students[i].last_name, last_name) == 0) {
            return i; // return index of the student
        }
    }
    return -1; // student not found
}

// searching by program of study
int search_student_by_program(student *students, int num_students, const char *program_study) {
    for (int i = 0; i < num_students; i++) {
        if (strcmp(students[i].program_study, program_study) == 0) {
            return i; // return index of the student
        }
    }
    return -1; // student not found
}