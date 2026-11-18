#include "main.h"

// Academic performance analysis
int academic_performance_analysis(student *students, int num_students) {
    int total_courses = 0;
    float total_marks = 0.0;
    float highest_average = -1.0;
    float lowest_average = 101.0;
    int passing_students = 0;
    int failing_students = 0;

    for (int i = 0; i < num_students; i++) {
        float student_total_marks = 0.0;
        for (int j = 0; j < students[i].number_of_courses; j++) {
            student_total_marks += students[i].performance.courses[j].marks_obtained;
            total_courses++;
        }
        float student_average = (students[i].number_of_courses > 0) ? (student_total_marks / students[i].number_of_courses) : 0.0;
        total_marks += student_total_marks;

        if (student_average > highest_average) {
            highest_average = student_average;
        }
        if (student_average < lowest_average) {
            lowest_average = student_average;
        }
        if (student_average >= PASSING_GRADE) {
            passing_students++;
        } else {
            failing_students++;
        }
    }

    float overall_average = (total_courses > 0) ? (total_marks / total_courses) : 0.0;

    printf("Total number of registered students: %d\n", num_students);
    printf("Total number of courses registered across all students: %d\n", total_courses);
    printf("Overall average academic performance: %.2f\n", overall_average);
    printf("Highest academic average: %.2f\n", highest_average);
    printf("Lowest academic average: %.2f\n", lowest_average);
    printf("Number of students passing: %d\n", passing_students);
    printf("Number of students failing: %d\n", failing_students);

    return 1; // success
}
