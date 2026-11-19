#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _POSIX_C_SOURCE 200809L   // for getline() and fileno-free POSIX APIs

#define REPORT_DIR "report"
#define PATH_LEN   512
#define ERR_LEN    256

typedef enum {
    STATUS_PENDING,           
    STATUS_COMPLETED,    
    STATUS_FAILED,         
    STATUS_THREAD_NOT_CREATED 
} ThreadStatus;

/* Everything one thread needs (input) and produces (results). */
typedef struct {
    // input
    int         thread_id;               
    const char *filename;                // file to analyse
    const char *keyword;                 // search keyword
    char        report_path[PATH_LEN];   // where this thread writes report

    /* ---- results ---- */
    size_t  total_lines;
    size_t  total_occurrences;
    size_t  matching_lines;              // number of entries in line_numbers
    size_t *line_numbers;                // dynamically grown array
    size_t  line_capacity;
    ThreadStatus status;
    char    error_message[ERR_LEN];
} ThreadArgs;

// Helpers 

static void set_error(ThreadArgs *a, const char *what, int err) {
    if (err != 0)
        snprintf(a->error_message, ERR_LEN, "%s: %s", what, strerror(err));
    else
        snprintf(a->error_message, ERR_LEN, "%s", what);
}

// Builds "report/<name without extension>_report.txt". Returns 0 or -1.
static int build_report_path(ThreadArgs *a) {
    const char *base = strrchr(a->filename, '/');
    base = base ? base + 1 : a->filename;

    char stem[PATH_LEN];
    snprintf(stem, sizeof stem, "%s", base);
    char *dot = strrchr(stem, '.');
    if (dot != NULL && dot != stem) *dot = '\0';

    int n = snprintf(a->report_path, PATH_LEN, "%s/%s_report.txt", REPORT_DIR, stem);
    return (n < 0 || n >= PATH_LEN) ? -1 : 0;
}

// Initialise the argument structure for one thread.
static void init_thread_args(ThreadArgs *a, int id, const char *filename, const char *keyword) {
    memset(a, 0, sizeof *a);
    a->thread_id = id;
    a->filename  = filename;
    a->keyword   = keyword;
    a->status    = STATUS_PENDING;
    if (build_report_path(a) != 0) {
        snprintf(a->report_path, PATH_LEN, "%s/thread_%d_report.txt", REPORT_DIR, id);
    }
}

// Remember that 'line_no' contains the keyword. Returns 0 or -1 (no memory).
static int record_match(ThreadArgs *a, size_t line_no) {
    if (a->matching_lines == a->line_capacity) {
        size_t new_cap = a->line_capacity ? a->line_capacity * 2 : 16;
        size_t *tmp = realloc(a->line_numbers, new_cap * sizeof *tmp);
        if (tmp == NULL) return -1;
        a->line_numbers = tmp;
        a->line_capacity = new_cap;
    }
    a->line_numbers[a->matching_lines++] = line_no;
    return 0;
}

// Searching                                                          
// Counts non-overlapping, case-insensitive occurrences of kw in line.
static size_t count_occurrences(const char *line, const char *kw, size_t klen) {
    size_t count = 0;
    const char *p = line;
    while (*p != '\0') {
        size_t i = 0;
        while (i < klen && p[i] != '\0' &&
               tolower((unsigned char)p[i]) == tolower((unsigned char)kw[i]))
            i++;
        if (i == klen) { count++; p += klen; }
        else           { p++; }
    }
    return count;
}

// Reads the file line by line
static int search_file(FILE *fp, ThreadArgs *a) {
    size_t klen = strlen(a->keyword);
    char *line = NULL;
    size_t cap = 0;

    while (getline(&line, &cap, fp) != -1) {
        a->total_lines++;
        size_t occ = count_occurrences(line, a->keyword, klen);
        if (occ > 0) {
            a->total_occurrences += occ;
            if (record_match(a, a->total_lines) != 0) {
                set_error(a, "memory allocation failed", 0);
                free(line);
                return -1;
            }
        }
    }
    free(line);

    if (ferror(fp)) {
        set_error(a, "error while reading file", errno);
        return -1;
    }
    return 0;
}

// Report generation
static int generate_report(const ThreadArgs *a) {
    FILE *out = fopen(a->report_path, "w");
    if (out == NULL) return -1;

    fprintf(out, "===== DOCUMENT ANALYSIS REPORT =====\n");
    fprintf(out, "Input file          : %s\n", a->filename);
    fprintf(out, "Search keyword      : %s (case-insensitive)\n", a->keyword);
    if (a->status == STATUS_COMPLETED)
        fprintf(out, "Processing status   : SUCCESS\n");
    else
        fprintf(out, "Processing status   : FAILED (%s)\n", a->error_message);
    fprintf(out, "Total lines         : %zu\n", a->total_lines);
    fprintf(out, "Total occurrences   : %zu\n", a->total_occurrences);
    fprintf(out, "Matching lines      : %zu\n", a->matching_lines);

    fprintf(out, "Matching line no.   : ");
    if (a->matching_lines == 0) {
        fprintf(out, "none\n");
        if (a->status == STATUS_COMPLETED)
            fprintf(out, "Note                : keyword not found in this file\n");
    } else {
        for (size_t i = 0; i < a->matching_lines; i++)
            fprintf(out, "%s%zu", i ? ", " : "", a->line_numbers[i]);
        fprintf(out, "\n");
    }

    if (ferror(out)) { fclose(out); return -1; }
    return fclose(out) == 0 ? 0 : -1;
}

// Thread function
static void *process_file(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;
    printf("[Thread %d] Processing %s ...\n", a->thread_id, a->filename);

    FILE *fp = fopen(a->filename, "r");
    if (fp == NULL) {
        set_error(a, "cannot open input file", errno);
        a->status = STATUS_FAILED;
    } else {
        a->status = (search_file(fp, a) == 0) ? STATUS_COMPLETED : STATUS_FAILED;
        fclose(fp);
    }

    // A report is written even for failures, so the problem is documented.
    if (generate_report(a) != 0) {
        set_error(a, "cannot write report file", errno);
        a->status = STATUS_FAILED;
    }

    if (a->status == STATUS_COMPLETED)
        printf("[Thread %d] Successfully processed %s\n", a->thread_id, a->filename);
    else
        printf("[Thread %d] ERROR processing %s: %s\n", a->thread_id, a->filename, a->error_message);
    return NULL;
}


// Result display
// Prints the summary. Returns the number of files that did not complete.
static int display_results(const ThreadArgs *args, int n) {
    int failures = 0;
    printf("\n===== PROCESSING SUMMARY =====\n");
    for (int i = 0; i < n; i++) {
        const ThreadArgs *a = &args[i];
        switch (a->status) {
        case STATUS_COMPLETED:
            printf("%s: Completed (%zu occurrence(s) in %zu of %zu line(s)%s)\n",
                   a->filename, a->total_occurrences, a->matching_lines, a->total_lines,
                   a->total_occurrences == 0 ? ", keyword not found" : "");
            break;
        case STATUS_THREAD_NOT_CREATED:
            printf("%s: Failed - thread could not be created\n", a->filename);
            failures++;
            break;
        default:
            printf("%s: Failed - %s\n", a->filename, a->error_message);
            failures++;
            break;
        }
    }
    if (failures == 0) {
        printf("All processing tasks completed.\nReports generated successfully in '%s/'.\n", REPORT_DIR);
    } else {
        printf("%d of %d task(s) failed. Reports for the other files are in '%s/'.\n",
               failures, n, REPORT_DIR);
    }
    return failures;
}

// main
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s \"keyword\" file1.txt [file2.txt ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *keyword = argv[1];
    if (keyword[0] == '\0') {
        fprintf(stderr, "Error: the search keyword must not be empty.\n");
        return EXIT_FAILURE;
    }
    int n = argc - 2;

    if (mkdir(REPORT_DIR, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: cannot create report directory '%s': %s\n", REPORT_DIR, strerror(errno));
        return EXIT_FAILURE;
    }

    ThreadArgs *args    = calloc((size_t)n, sizeof *args);
    pthread_t  *threads = calloc((size_t)n, sizeof *threads);
    int        *created = calloc((size_t)n, sizeof *created);
    if (!args || !threads || !created) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        free(args); free(threads); free(created);
        return EXIT_FAILURE;
    }

    printf("===== MULTI-THREADED DOCUMENT ANALYZER =====\n");
    printf("Search Keyword: %s\n", keyword);
    printf("Number of Input Files: %d\n", n);

    /* One thread per file */
    for (int i = 0; i < n; i++) {
        init_thread_args(&args[i], i + 1, argv[i + 2], keyword);
        int rc = pthread_create(&threads[i], NULL, process_file, &args[i]);
        if (rc != 0) {
            fprintf(stderr, "[Thread %d] ERROR: pthread_create failed for %s: %s\n",
                    args[i].thread_id, args[i].filename, strerror(rc));
            args[i].status = STATUS_THREAD_NOT_CREATED;
        } else {
            created[i] = 1;
        }
    }

    /* Wait for every thread that was actually created */
    for (int i = 0; i < n; i++)
        if (created[i]) pthread_join(threads[i], NULL);

    int failures = display_results(args, n);

    for (int i = 0; i < n; i++) free(args[i].line_numbers);
    free(args); free(threads); free(created);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}