#include "main.h"

// register a new student
void register_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file) {
    if (*num_students >= 100) {
        printf("Maximum number of students reached.\n");
        return;
    }
    student new_student;
    printf("Enter student ID: ");
    scanf("%d", &new_student.id);
    printf("Enter first name: ");
    scanf("%s", new_student.first_name);
    printf("Enter last name: ");
    scanf("%s", new_student.last_name);
    printf("Enter program of study: ");
    scanf("%s", new_student.program_study);
    printf("Enter year of study: ");
    scanf("%d", &new_student.year_of_study);

    // display the list of courses and let the user select courses or enter new courses from the file of courses
    while(fgets(line, sizeof(line), course_file) != NULL){
        printf("%s", line);
    }
    printf("are you sure you want to select these courses? (y/n): ");
    if(getchar() == 'y') {
        // read the selected courses from the file and store them in the student's academic performance
        for (int i = 0; i < new_student.number_of_courses; i++) {
            fgets(line, sizeof(line), course_file);
            sscanf(line, "%s %s %f %f", new_student.performance.courses[i].course_code, new_student.performance.courses[i].course_name, &new_student.performance.courses[i].marks_obtained, &new_student.performance.courses[i].maximum_marks);
        }
    } else {
        printf("Enter the number of courses: ");
        scanf("%d", &new_student.number_of_courses);
        for (int i = 0; i < new_student.number_of_courses; i++) {
            printf("Enter course code: ");
            scanf("%s", new_student.performance.courses[i].course_code);
            printf("Enter course name: ");
            scanf("%s", new_student.performance.courses[i].course_name);
            printf("Enter marks obtained: ");
            scanf("%f", &new_student.performance.courses[i].marks_obtained);
            printf("Enter maximum marks: ");
            scanf("%f", &new_student.performance.courses[i].maximum_marks);
        }
    }
}

int update_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file) {
    int id;
    printf("Enter student ID to update: ");
    scanf("%d", &id);
    for (int i = 0; i < *num_students; i++) {
        if (students[i].id == id) {
            printf("Enter new first name: ");
            scanf("%s", students[i].first_name);
            printf("Enter new last name: ");
            scanf("%s", students[i].last_name);
            printf("Enter new program of study: ");
            scanf("%s", students[i].program_study);
            printf("Enter new year of study: ");
            scanf("%d", &students[i].year_of_study);
            return 1; // success
        }
    }
    printf("Student with ID %d not found.\n", id);
    return 0; // failure
}

int delete_student(student *students, int *num_students, FILE *student_file, FILE *course_file, FILE *grade_file) {
    int id;
    printf("Enter student ID to delete: ");
    scanf("%d", &id);
    for (int i = 0; i < *num_students; i++) {
        if (students[i].id == id) {
            // Shift all students after the deleted one to the left
            for (int j = i; j < *num_students - 1; j++) {
                students[j] = students[j + 1];
            }
            (*num_students)--;
            return 1; // success
        }
    }
    printf("Student with ID %d not found.\n", id);
    return 0; // failure
}

// add a course to a student's academic performance
void add_course(student *students, int num_students, FILE *course_file) {
    int id;
    printf("Enter student ID to add a course: ");
    scanf("%d", &id);
    for (int i = 0; i < num_students; i++) {
        if (students[i].id == id) {
            if (students[i].number_of_courses >= MAX_COURSES) {
                printf("Maximum number of courses reached for this student.\n");
                return;
            }
            printf("Enter course code: ");
            scanf("%s", students[i].performance.courses[students[i].number_of_courses].course_code);
            printf("Enter course name: ");
            scanf("%s", students[i].performance.courses[students[i].number_of_courses].course_name);
            printf("Enter marks obtained: ");
            scanf("%f", &students[i].performance.courses[students[i].number_of_courses].marks_obtained);
            printf("Enter maximum marks: ");
            scanf("%f", &students[i].performance.courses[students[i].number_of_courses].maximum_marks);
            students[i].number_of_courses++;
            return;
        }
    }
    printf("Student with ID %d not found.\n", id);
}

// Display a student's academic performance
void display_academic_performance(student *students, int num_students) {
    int id;
    printf("Enter student ID to display academic performance: ");
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