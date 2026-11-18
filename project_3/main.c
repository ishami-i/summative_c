#include "main.h"

// Define the global file pointers
FILE *student_file = NULL;
FILE *course_file = NULL;
FILE *grade_file = NULL;

int main() {
    // Open the files inside a function
    student_file = fopen("data/students.txt", "rb+");
    course_file = fopen("data/courses.txt", "rb+");
    grade_file = fopen("data/grades.txt", "rb+");

    if (!student_file || !course_file || !grade_file) {
        printf("Error opening one or more files.\n");
        return 1;
    }
    /*
    the choices avaiable to the user are with function pointers and the user can choose to perform any of the following operations:
    Registering students.

Adding and updating course records.

Displaying student information.

Deleting student records.

Searching for students.

Sorting student records.

Calculating academic averages.

Generating academic reports.

Saving records to files.

Loading records from files.

Validating user input.
    */

    // Close files before exiting
    fclose(student_file);
    fclose(course_file);
    fclose(grade_file);

    return 0;
}