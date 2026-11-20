#include "main.h"

// Menu table: each entry pairs a label with the function that handles it. 
static const MenuItem MENU[] = {
    { "Register a new student",              register_student },
    { "Display all students",                display_all_students },
    { "View detailed student information",   view_student },
    { "Update student information",          update_student },
    { "Delete a student record",             delete_student },
    { "Add a course to a student",           add_course },
    { "Update marks for a course",           update_marks },
    { "Display a student's performance",     display_student_performance },
    { "Search students",                     search_students },
    { "Sort students",                       sort_students },
    { "Generate academic report",            generate_report },
    { "Save records to file",                save_records },
    { "Load records from file",              load_records },
};
#define MENU_SIZE ((int)(sizeof MENU / sizeof MENU[0]))

static void print_menu(const StudentDB *db) {
    printf("\n=========== Student Academic Records System ===========\n");
    for (int i = 0; i < MENU_SIZE; i++) printf("  %2d. %s\n", i + 1, MENU[i].label);
    printf("   0. Exit\n");
    printf("-------------------------------------------------------\n");
    printf("Students in memory: %d%s\n", db->count, db->modified ? "   (unsaved changes)" : "");
}

int main(void) {
    StudentDB db;

    if (db_init(&db) != OK) {
        fprintf(stderr, "Fatal: %s.\n", status_message(ERR_MEMORY));
        return 1;
    }

    // load previously saved records; a missing file just means a fresh start
    Status st = read_records_file(&db);
    if (st != OK && st != ERR_NOT_FOUND)
        printf("Starting with an empty database. Note: saving will overwrite %s.\n", DATA_FILE);

    for (;;) {
        int choice;
        print_menu(&db);
        if (!read_int("Select an option: ", 0, MENU_SIZE, &choice)) break;
        if (choice == 0) break;
        MENU[choice - 1].handler(&db);
    }

    if (db.modified && confirm("You have unsaved changes. Save before exiting? (y/n): "))
        save_records(&db);

    db_free(&db);
    printf("Goodbye!\n");
    return 0;
}
