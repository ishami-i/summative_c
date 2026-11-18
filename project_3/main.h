#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COURSES 10
#define MAX_STUDENTS 100
#define PASSING_GRADE 50.0

// Declare file pointers as extern so multiple .c files can share them
extern FILE *student_file;
extern FILE *course_file;
extern FILE *grade_file;

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
// functions 
void register_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file);
int update_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file);
int delete_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file);

#endif // MAIN_H