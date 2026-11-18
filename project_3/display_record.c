#include "main.h"

// display a student's record
void display_student_record(student *students, int num_students) {
    int id;
    printf("Enter student ID to display: ");
    scanf("%d", &id);
    for (int i = 0; i < num_students; i++) {
        if (students[i].id == id) {
            printf("Student ID: %d\n", students[i].id);
            printf("First Name: %s\n", students[i].first_name);
            printf("Last Name: %s\n", students[i].last_name);
            printf("Program of Study: %s\n", students[i].program_study);
            printf("Year of Study: %d\n", students[i].year_of_study);
            printf("Number of Courses: %d\n", students[i].number_of_courses);
            for (int j = 0; j < students[i].number_of_courses; j++) {
                printf("Course Code: %s\n", students[i].performance.courses[j].course_code);
                printf("Course Name: %s\n", students[i].performance.courses[j].course_name);
                printf("Marks Obtained: %.2f\n", students[i].performance.courses[j].marks_obtained);
                printf("Maximum Marks: %.2f\n", students[i].performance.courses[j].maximum_marks);
            }
            return;
        }
    }
    printf("Student with ID %d not found.\n", id);
}
void display_all_students(student *students, int num_students) {
    for (int i = 0; i < num_students; i++) {
        printf("Student ID: %d\n", students[i].id);
        printf("First Name: %s\n", students[i].first_name);
        printf("Last Name: %s\n", students[i].last_name);
        printf("Program of Study: %s\n", students[i].program_study);
        printf("Year of Study: %d\n", students[i].year_of_study);
        printf("Number of Courses: %d\n", students[i].number_of_courses);
        for (int j = 0; j < students[i].number_of_courses; j++) {
            printf("Course Code: %s\n", students[i].performance.courses[j].course_code);
            printf("Course Name: %s\n", students[i].performance.courses[j].course_name);
            printf("Marks Obtained: %.2f\n", students[i].performance.courses[j].marks_obtained);
            printf("Maximum Marks: %.2f\n", students[i].performance.courses[j].maximum_marks);
        }
        printf("\n");
    }
}