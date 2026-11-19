#include "main.h"

// predicates (callbacks)
int pred_amount_above(const Transaction *t, const void *ctx) {
    return t->transaction_amount > *(const double *)ctx;
}

int pred_amount_equals(const Transaction *t, const void *ctx) {
    double diff = t->transaction_amount - *(const double *)ctx;
    return diff > -0.005 && diff < 0.005;
}

int pred_product_equals(const Transaction *t, const void *ctx) {
    return str_equal_ignore_case(t->product_name, (const char *)ctx);
}

int pred_quantity_at_least(const Transaction *t, const void *ctx) {
    return t->quantity >= *(const int *)ctx;
}

// functions that recieve and invoke the callbacks
size_t filter_transactions(const TransactionList *list, TransactionPredicate pred, const void *ctx) {
    size_t matches = 0;
    for (size_t i = 0; i < list->count; i++) {
        if (pred(&list->items[i], ctx)) {              
            if (matches == 0) { printf("\n"); print_header(); }
            print_transaction(&list->items[i]);
            matches++;
        }
    }
    if (matches == 0) printf("No matching transactions found.\n");
    else printf("\n%zu matching transaction(s).\n", matches);
    return matches;
}

size_t count_transactions(const TransactionList *list, TransactionPredicate pred, const void *ctx) {
    size_t count = 0;
    for (size_t i = 0; i < list->count; i++)
        if (pred(&list->items[i], ctx)) count++;        
    return count;
}