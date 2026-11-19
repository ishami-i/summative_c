#include "main.h"

// dynamic array helpers
int list_init(TransactionList *list) {
    list->items = malloc(INITIAL_CAPACITY * sizeof(Transaction));
    if (list->items == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        list->count = list->capacity = 0;
        return -1;
    }
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    return 0;
}

int list_ensure_capacity(TransactionList *list) {
    if (list->count < list->capacity) return 0;
    size_t new_capacity = list->capacity ? list->capacity * 2 : INITIAL_CAPACITY;
    Transaction *tmp = realloc(list->items, new_capacity * sizeof(Transaction));
    if (tmp == NULL) {                         /* old block is still valid */
        fprintf(stderr, "Error: memory allocation failed.\n");
        return -1;
    }
    list->items = tmp;
    list->capacity = new_capacity;
    return 0;
}

int list_append(TransactionList *list, const Transaction *t) {
    if (list_ensure_capacity(list) != 0) return -1;
    list->items[list->count++] = *t;
    return 0;
}

void list_free(TransactionList *list) {
    free(list->items);
    list->items = NULL;
    list->count = list->capacity = 0;
}

int find_index_by_id(const TransactionList *list, int id) {
    for (size_t i = 0; i < list->count; i++)
        if (list->items[i].transaction_id == id) return (int)i;
    return -1;
}

// input helpers with validation
static int ask_positive_int(const char *prompt, int *out) {
    for (;;) {
        int v, r = read_int(prompt, &v);
        if (r == -1) return 0;
        if (r == 1 && v > 0) { *out = v; return 1; }
        printf("Invalid input: enter a whole number greater than 0.\n");
    }
}

static int ask_positive_double(const char *prompt, double *out) {
    for (;;) {
        double v;
        int r = read_double(prompt, &v);
        if (r == -1) return 0;
        if (r == 1 && v > 0.0) { *out = v; return 1; }
        printf("Invalid input: enter a price greater than 0.\n");
    }
}

static int ask_product(char *out) {
    for (;;) {
        int r = read_string("Enter Product Name: ", out, PRODUCT_NAME_LEN);
        if (r == -1) return 0;
        if (r == 1) return 1;
        printf("Invalid name: 1-%d characters, and no '|' character.\n", PRODUCT_NAME_LEN - 1);
    }
}

static int ask_date(char *out) {
    char buf[64];
    for (;;) {
        if (!read_line("Enter Date YYYY-MM-DD (Enter = today): ", buf, sizeof buf)) return 0;
        if (buf[0] == '\0') { get_today(out, DATE_LEN); return 1; }
        if (is_valid_date(buf)) { memcpy(out, buf, DATE_LEN); return 1; }
        printf("Invalid date. Use the format YYYY-MM-DD.\n");
    }
}

// Asks product, quantity, price, date; computes total. Returns 1 on success, 0 if cancelled.
static int ask_details(Transaction *t) {
    if (!ask_product(t->product_name)) return 0;
    if (!ask_positive_int("Enter Quantity: ", &t->quantity)) return 0;
    if (!ask_positive_double("Enter Unit Price: ", &t->unit_price)) return 0;
    if (!ask_date(t->transaction_date)) return 0;
    t->transaction_amount = t->quantity * t->unit_price;
    return 1;
}

// CRUD
int add_transaction(TransactionList *list) {
    Transaction t;
    memset(&t, 0, sizeof t);

    for (;;) {
        int r = read_int("Enter Transaction ID: ", &t.transaction_id);
        if (r == -1) return -1;
        if (r != 1 || t.transaction_id <= 0) {
            printf("Invalid ID: enter a whole number greater than 0.\n");
        } else if (find_index_by_id(list, t.transaction_id) != -1) {
            printf("Error: Transaction ID %d already exists. Choose a different ID.\n", t.transaction_id);
        } else break;
    }

    if (!ask_details(&t)) { printf("Cancelled.\n"); return -1; }
    if (list_append(list, &t) != 0) return -1;

    printf("Transaction added successfully! (Total: %.2f)\n", t.transaction_amount);
    return 0;
}

void display_transactions(const TransactionList *list) {
    if (list->count == 0) {
        printf("No transactions recorded.\n");
        return;
    }
    printf("\n");
    print_header();
    for (size_t i = 0; i < list->count; i++) print_transaction(&list->items[i]);
    printf("\n%zu transaction(s).\n", list->count);
}

int update_transaction(TransactionList *list) {
    if (list->count == 0) { printf("No transactions recorded.\n"); return -1; }

    int id, r = read_int("Enter Transaction ID to update: ", &id);
    if (r != 1) { printf("Invalid ID.\n"); return -1; }

    int idx = find_index_by_id(list, id);
    if (idx < 0) { printf("Transaction ID %d not found.\n", id); return -1; }

    printf("Current record:\n");
    print_header();
    print_transaction(&list->items[idx]);

    Transaction updated = list->items[idx];
    if (!ask_details(&updated)) { printf("Update cancelled.\n"); return -1; }
    list->items[idx] = updated;

    printf("Transaction updated successfully!\n");
    return 0;
}

int delete_transaction(TransactionList *list) {
    if (list->count == 0) { printf("No transactions recorded.\n"); return -1; }

    int id, r = read_int("Enter Transaction ID to delete: ", &id);
    if (r != 1) { printf("Invalid ID.\n"); return -1; }

    int idx = find_index_by_id(list, id);
    if (idx < 0) { printf("Transaction ID %d not found.\n", id); return -1; }

    // shift remaining records left to fill the gap
    memmove(&list->items[idx], &list->items[idx + 1],
            (list->count - (size_t)idx - 1) * sizeof(Transaction));
    list->count--;

    printf("Transaction %d deleted successfully!\n", id);
    return 0;
}