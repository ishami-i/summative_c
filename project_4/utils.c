#include "main.h"
#include <errno.h>
#include <limits.h>
#include <math.h>

int read_line(const char *prompt, char *buf, size_t size) {
    if (prompt) { printf("%s", prompt); fflush(stdout); }
    if (fgets(buf, (int)size, stdin) == NULL) return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {                                  
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    return 1;
}

int read_int(const char *prompt, int *out) {
    char buf[64], *end;
    if (!read_line(prompt, buf, sizeof buf)) return -1;
    errno = 0;
    long v = strtol(buf, &end, 10);
    if (end == buf || errno == ERANGE || v > INT_MAX || v < INT_MIN) return 0;
    while (isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;
    *out = (int)v;
    return 1;
}

int read_double(const char *prompt, double *out) {
    char buf[64], *end;
    if (!read_line(prompt, buf, sizeof buf)) return -1;
    errno = 0;
    double v = strtod(buf, &end);
    if (end == buf || errno == ERANGE || !isfinite(v)) return 0;
    while (isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;
    *out = v;
    return 1;
}

// Reads a non-empty string that fits in 'size' and contains no '|' 
int read_string(const char *prompt, char *out, size_t size) {
    char buf[128];
    if (!read_line(prompt, buf, sizeof buf)) return -1;
    char *start = buf;
    while (isspace((unsigned char)*start)) start++;
    size_t len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) start[--len] = '\0';
    if (len == 0 || len >= size || strchr(start, '|') != NULL) return 0;
    memcpy(out, start, len + 1);
    return 1;
}

void get_today(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL || strftime(buf, size, "%Y-%m-%d", t) == 0)
        snprintf(buf, size, "0000-00-00");
}

int is_valid_date(const char *s) {
    if (strlen(s) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) { if (s[i] != '-') return 0; }
        else if (!isdigit((unsigned char)s[i])) return 0;
    }
    int month = (s[5] - '0') * 10 + (s[6] - '0');
    int day   = (s[8] - '0') * 10 + (s[9] - '0');
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

int compare_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a), cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int str_equal_ignore_case(const char *a, const char *b) {
    return compare_ignore_case(a, b) == 0;
}

void print_header(void) {
    printf("%-6s %-20s %8s %12s %14s  %s\n",
           "ID", "Product", "Qty", "Unit Price", "Total", "Date");
    printf("------------------------------------------------------------------------\n");
}

void print_transaction(const Transaction *t) {
    printf("%-6d %-20s %8d %12.2f %14.2f  %s\n",
           t->transaction_id, t->product_name, t->quantity,
           t->unit_price, t->transaction_amount, t->transaction_date);
}