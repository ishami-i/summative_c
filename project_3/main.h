#ifndef MAIN_H
#define MAIN_H

#define MAX_COURSES 10

// Define course first
typedef struct {
    char course_code[10];
    char course_name[50];
    float marks_obtained;
    float maximum_marks;
} course;

// Define academic_performance second
typedef struct {
    course courses[MAX_COURSES];
} academic_performance;

// Define student
typedef struct {
    int id;
    char first_name[50];
    char last_name[50];
    char program_study[50];
    int year_of_study;
    int number_of_courses;
    academic_performance performance;
} student;

#endif // MAIN_H
