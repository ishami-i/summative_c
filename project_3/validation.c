#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

// Text helpers  

// Remove leading and trailing whitespace in place.
void trim_string(char *s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    size_t start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
}

/* Case-insensitive strcmp. */
int str_icmp(const char *a, const char *b) {
    while (*a && *b) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0) return d;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

// Case-insensitive substring test (manual implementation, no strstr). 
int str_icontains(const char *haystack, const char *needle) {
    size_t n = strlen(needle);
    if (n == 0) return 1;
    for (; *haystack; haystack++) {
        size_t i = 0;
        while (i < n && haystack[i] &&
               tolower((unsigned char)haystack[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == n) return 1;
    }
    return 0;
}

// Non-empty, shorter than `size`, no separator and no control characters. 
int is_valid_text(const char *s, size_t size) {
    size_t len = strlen(s);
    if (len == 0 || len >= size) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 32 || c == 127 || c == FIELD_SEP) return 0;
    }
    return !isspace((unsigned char)s[0]) && !isspace((unsigned char)s[len - 1]);
}

// Letters and digits only, 1..CODE_LEN-1 characters. 
int is_valid_course_code(const char *s) {
    size_t len = strlen(s);
    if (len == 0 || len >= CODE_LEN) return 0;
    for (size_t i = 0; i < len; i++)
        if (!isalnum((unsigned char)s[i])) return 0;
    return 1;
}

// Safe input (fgets based: no scanf, no leftover newlines)        

// Reads one line. Returns 1 on success, 0 on end of input (EOF). 
int read_line(const char *prompt, char *buf, size_t size) {
    static int interactive = -1;
    if (interactive < 0) interactive = isatty(STDIN_FILENO);

    if (prompt) { printf("%s", prompt); fflush(stdout); }
    if (!fgets(buf, (int)size, stdin)) return 0;

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {                              /* line longer than the buffer: discard the rest */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    trim_string(buf);
    if (!interactive) printf("%s\n", buf);   /* echo so scripted runs are readable */
    return 1;
}

// Reads an integer in [min, max]; repeats until valid. Returns 0 on EOF.
int read_int(const char *prompt, int min, int max, int *out) {
    char buf[64];
    for (;;) {
        if (!read_line(prompt, buf, sizeof buf)) return 0;
        char *end;
        errno = 0;
        long v = strtol(buf, &end, 10);
        if (buf[0] == '\0' || *end != '\0' || errno == ERANGE || v < min || v > max) {
            printf("  Invalid input. Enter a whole number between %d and %d.\n", min, max);
            continue;
        }
        *out = (int)v;
        return 1;
    }
}

// Reads a real number in [min, max]; rejects text, nan and inf. Returns 0 on EOF. 
int read_float(const char *prompt, double min, double max, float *out) {
    char buf[64];
    for (;;) {
        if (!read_line(prompt, buf, sizeof buf)) return 0;
        char *end;
        errno = 0;
        double v = strtod(buf, &end);
        if (buf[0] == '\0' || *end != '\0' || errno == ERANGE || !(v >= min && v <= max)) {
            printf("  Invalid input. Enter a number between %g and %g.\n", min, max);
            continue;
        }
        *out = (float)v;
        return 1;
    }
}

// Reads text (spaces allowed) that fits in `size` bytes. Returns 0 on EOF. 
int read_text(const char *prompt, char *buf, size_t size) {
    char tmp[256];
    for (;;) {
        if (!read_line(prompt, tmp, sizeof tmp)) return 0;
        if (!is_valid_text(tmp, size)) {
            printf("  Invalid text. Use 1-%zu characters and avoid the '%c' character.\n",
                   size - 1, FIELD_SEP);
            continue;
        }
        snprintf(buf, size, "%s", tmp);
        return 1;
    }
}

// Reads a course code and converts it to upper case. Returns 0 on EOF.
int read_course_code(const char *prompt, char *buf, size_t size) {
    char tmp[64];
    for (;;) {
        if (!read_line(prompt, tmp, sizeof tmp)) return 0;
        for (char *p = tmp; *p; p++) *p = (char)toupper((unsigned char)*p);
        if (!is_valid_course_code(tmp) || strlen(tmp) >= size) {
            printf("  Invalid course code. Use letters/digits only, 1-%d characters.\n", CODE_LEN - 1);
            continue;
        }
        snprintf(buf, size, "%s", tmp);
        return 1;
    }
}

// y/n question. Returns 1 only for an explicit yes.
int confirm(const char *prompt) {
    char buf[16];
    if (!read_line(prompt, buf, sizeof buf)) return 0;
    return buf[0] == 'y' || buf[0] == 'Y';
}
