#include "main.h"

// callbacks for sorting
int cmp_by_id(const Transaction *a, const Transaction *b) {
    return (a->transaction_id > b->transaction_id) - (a->transaction_id < b->transaction_id);
}

int cmp_by_product(const Transaction *a, const Transaction *b) {
    return compare_ignore_case(a->product_name, b->product_name);
}

int cmp_by_amount_asc(const Transaction *a, const Transaction *b) {
    return (a->transaction_amount > b->transaction_amount) - (a->transaction_amount < b->transaction_amount);
}

int cmp_by_amount_desc(const Transaction *a, const Transaction *b) {
    return cmp_by_amount_asc(b, a);
}

//  Manual insertion sort
void sort_transactions(TransactionList *list, TransactionComparator cmp) {
    for (size_t i = 1; i < list->count; i++) {
        Transaction key = list->items[i];
        size_t j = i;
        while (j > 0 && cmp(&list->items[j - 1], &key) > 0) {
            list->items[j] = list->items[j - 1];
            j--;
        }
        list->items[j] = key;
    }
}

// linear searching
void search_by_id(const TransactionList *list) {
    int id, r = read_int("Enter Transaction ID to search: ", &id);
    if (r != 1) { printf("Invalid ID.\n"); return; }

    int idx = find_index_by_id(list, id);
    if (idx < 0) { printf("Transaction ID %d not found.\n", id); return; }
    printf("\n");
    print_header();
    print_transaction(&list->items[idx]);
}

void search_by_product(const TransactionList *list) {
    char product[PRODUCT_NAME_LEN];
    int r = read_string("Enter Product Name to search: ", product, sizeof product);
    if (r != 1) { printf("Invalid product name.\n"); return; }
    filter_transactions(list, pred_product_equals, product);
}

void search_by_amount(const TransactionList *list) {
    double amount;
    int r = read_double("Enter Transaction Amount to search: ", &amount);
    if (r != 1) { printf("Invalid amount.\n"); return; }
    filter_transactions(list, pred_amount_equals, &amount);
}