// week5_task3_student_management_system.c
// Name: Kayra Aslan  Surname: Oncul  Student ID: 241ADB119
//
// Task 3: Mini-project – Student management system with file persistence
// Week 5 – Files & Modular Programming
//
// Build: make lab5   Run: ./bin/week5_task3_student_management_system

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define NAME_LEN 50
#define LINE_LEN 256
#define DATA_FILE "students.txt"

typedef struct Student {
    char name[NAME_LEN];
    int id;
    float gpa;
} Student;

// Function prototypes (do not change these signatures – the autograder calls them directly)
int  load_students(Student arr[], int max);          // returns number of records loaded
int  save_students(const Student arr[], int count);  // 0 on success, -1 on error
void add_student(Student arr[], int *count);
void list_students(const Student arr[], int count);
int  read_line(const char *prompt, char *buf, size_t size);

int main(void) {
    Student students[MAX_STUDENTS];
    int count = 0;
    char input[LINE_LEN];
    int running = 1;
    int status = EXIT_SUCCESS;

    // Load existing data (0 on first run)
    count = load_students(students, MAX_STUDENTS);
    printf("Loaded %d student(s) from %s\n", count, DATA_FILE);

    while (running) {
        printf("\n=== Student Management System ===\n");
        printf("1. List students\n");
        printf("2. Add student\n");
        printf("3. Save and Exit\n");

        int choice;
        if (read_line("Select an option: ", input, sizeof input) != 0) {
            choice = 3;                         // end of input: save and exit instead of looping forever
        } else {
            char *end;
            long value = strtol(input, &end, 10);
            choice = (end != input && *end == '\0') ? (int)value : 0;   // non-numbers become invalid
        }

        switch (choice) {
            case 1:
                list_students(students, count);
                break;
            case 2:
                add_student(students, &count);
                break;
            case 3:
                if (save_students(students, count) != 0) {
                    fprintf(stderr, "Error: could not save data to %s\n", DATA_FILE);
                    status = EXIT_FAILURE;
                } else {
                    printf("Data saved. Goodbye!\n");
                }
                running = 0;
                break;
            default:
                printf("Invalid option. Try again.\n");
        }
    }

    return status;
}

// Provided helper – prints a prompt and reads one line from stdin into buf.
// The trailing newline is removed. Returns 0 on success, -1 on end of input (EOF).
int read_line(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)size, stdin) == NULL) {
        return -1;
    }
    size_t len = strcspn(buf, "\r\n");
    if (buf[len] == '\0' && len == size - 1) {
        // Line was longer than the buffer: discard the rest of it.
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    buf[len] = '\0';
    return 0;
}

// Loads records from DATA_FILE. Missing file (first run) or empty file -> 0.
// Blank/malformed lines are skipped; never more than max records are stored.
int load_students(Student arr[], int max) {
    if (arr == NULL || max <= 0) {
        return 0;
    }

    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        return 0;                               // not an error: nothing saved yet
    }

    char line[LINE_LEN];
    int count = 0;

    while (count < max && fgets(line, sizeof line, fp) != NULL) {
        // If the line did not fit in the buffer, throw away the rest of it
        if (strchr(line, '\n') == NULL && !feof(fp)) {
            int c;
            while ((c = fgetc(fp)) != '\n' && c != EOF) { }
        }

        Student s = {0};
        // %49[^,] = up to 49 chars that are not a comma (spaces allowed)
        if (sscanf(line, "%49[^,],%d,%f", s.name, &s.id, &s.gpa) == 3) {
            arr[count++] = s;
        }
        // otherwise: blank or malformed line -> skipped
    }

    fclose(fp);
    return count;
}

// Overwrites DATA_FILE with all students, one "name,id,gpa" line each.
int save_students(const Student arr[], int count) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        return -1;
    }

    int ok = 1;
    for (int i = 0; i < count; i++) {
        if (fprintf(fp, "%s,%d,%.2f\n", arr[i].name, arr[i].id, arr[i].gpa) < 0) {
            ok = 0;
            break;
        }
    }

    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok ? 0 : -1;
}

// Asks for name, ID and GPA, validates them and appends the student.
void add_student(Student arr[], int *count) {
    if (*count >= MAX_STUDENTS) {
        printf("Cannot add more students (limit reached).\n");
        return;
    }

    char name[LINE_LEN], id_str[LINE_LEN], gpa_str[LINE_LEN];

    // Read all three answers first so the input stays in sync even if one is invalid
    if (read_line("Enter name: ", name, sizeof name) != 0) return;
    if (read_line("Enter ID: ", id_str, sizeof id_str) != 0) return;
    if (read_line("Enter GPA: ", gpa_str, sizeof gpa_str) != 0) return;

    // Name: not empty/blank, no comma, fits in the array
    const char *p = name;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0' || strchr(name, ',') != NULL || strlen(name) >= NAME_LEN) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // ID: must be a whole number that fits in an int
    char *end;
    errno = 0;
    long id = strtol(id_str, &end, 10);
    if (end == id_str || *end != '\0' || errno == ERANGE || id < INT_MIN || id > INT_MAX) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // GPA: must be a number between 0.0 and 4.0 (NaN fails these comparisons too)
    float gpa = strtof(gpa_str, &end);
    if (end == gpa_str || *end != '\0' || !(gpa >= 0.0f && gpa <= 4.0f)) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    Student *s = &arr[*count];
    strcpy(s->name, name);                      // length already checked above
    s->id = (int)id;
    s->gpa = gpa;
    (*count)++;
    printf("Student added successfully!\n");
}

// Prints a table of all students, or a message if there are none.
void list_students(const Student arr[], int count) {
    if (count == 0) {
        printf("No students found.\n");
        return;
    }

    printf("%-5s %-20s %s\n", "ID", "Name", "GPA");
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %.2f\n", arr[i].id, arr[i].name, arr[i].gpa);
    }
}
