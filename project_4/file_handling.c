#include "main.h"
#include <errno.h>

// Text format, one record per line:  id|product|quantity|unit_price|total|date
int save_transactions(const TransactionList *list, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "Error: cannot open '%s' for writing: %s\n", filename, strerror(errno));
        return -1;
    }
    for (size_t i = 0; i < list->count; i++) {
        const Transaction *t = &list->items[i];
        if (fprintf(f, "%d|%s|%d|%.2f|%.2f|%s\n", t->transaction_id, t->product_name,
                    t->quantity, t->unit_price, t->transaction_amount, t->transaction_date) < 0) {
            fprintf(stderr, "Error: failed while writing to '%s'.\n", filename);
            fclose(f);
            return -1;
        }
    }
    if (fclose(f) != 0) {
        fprintf(stderr, "Error: failed to close '%s': %s\n", filename, strerror(errno));
        return -1;
    }
    printf("Saved %zu transaction(s) to '%s'.\n", list->count, filename);
    return 0;
}

static int parse_line(const char *line, Transaction *t) {
    double stored_total;
    memset(t, 0, sizeof *t);
    int n = sscanf(line, "%d|%49[^|]|%d|%lf|%lf|%10s",
                   &t->transaction_id, t->product_name, &t->quantity,
                   &t->unit_price, &stored_total, t->transaction_date);
    if (n != 6) return 0;
    if (t->transaction_id <= 0 || t->quantity <= 0 || t->unit_price <= 0.0) return 0;
    if (!is_valid_date(t->transaction_date)) return 0;
    t->transaction_amount = t->quantity * t->unit_price;
    return 1;
}

int load_transactions(TransactionList *list, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        if (errno == ENOENT) {
            printf("No saved file '%s' found. Starting fresh.\n", filename);
            return 0;
        }
        fprintf(stderr, "Error: cannot open '%s' for reading: %s\n", filename, strerror(errno));
        return -1;
    }

    // load into a temporary list first
    TransactionList loaded;
    if (list_init(&loaded) != 0) { fclose(f); return -1; }

    char line[256];
    int line_no = 0, skipped = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        line_no++;
        if (line[0] == '\n' || line[0] == '\0') continue;

        Transaction t;
        if (!parse_line(line, &t)) {
            fprintf(stderr, "Warning: skipped invalid record on line %d.\n", line_no);
            skipped++;
            continue;
        }
        if (find_index_by_id(&loaded, t.transaction_id) != -1) {
            fprintf(stderr, "Warning: skipped duplicate ID %d on line %d.\n", t.transaction_id, line_no);
            skipped++;
            continue;
        }
        if (list_append(&loaded, &t) != 0) {
            fclose(f);
            list_free(&loaded);
            return -1;
        }
    }

    if (ferror(f)) {
        fprintf(stderr, "Error: failed while reading '%s'.\n", filename);
        fclose(f);
        list_free(&loaded);
        return -1;
    }
    fclose(f);

    list_free(list);                     
    *list = loaded;
    printf("Loaded %zu transaction(s) from '%s'%s.\n", list->count, filename,
           skipped ? " (some lines were skipped)" : "");
    return 0;
}