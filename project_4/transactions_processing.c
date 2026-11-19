#include "main.h"

// calculations
double calculate_total_revenue(const TransactionList *list) {
    double total = 0.0;
    for (size_t i = 0; i < list->count; i++) total += list->items[i].transaction_amount;
    return total;
}

double calculate_average_transaction_value(const TransactionList *list) {
    if (list->count == 0) return 0.0;               
    return calculate_total_revenue(list) / (double)list->count;
}

const Transaction *find_highest_transaction(const TransactionList *list) {
    if (list->count == 0) return NULL;
    const Transaction *best = &list->items[0];
    for (size_t i = 1; i < list->count; i++)
        if (list->items[i].transaction_amount > best->transaction_amount) best = &list->items[i];
    return best;
}

const Transaction *find_lowest_transaction(const TransactionList *list) {
    if (list->count == 0) return NULL;
    const Transaction *best = &list->items[0];
    for (size_t i = 1; i < list->count; i++)
        if (list->items[i].transaction_amount < best->transaction_amount) best = &list->items[i];
    return best;
}

long calculate_total_units_sold(const TransactionList *list) {
    long units = 0;
    for (size_t i = 0; i < list->count; i++) units += list->items[i].quantity;
    return units;
}

double calculate_product_revenue(const TransactionList *list, const char *product) {
    double total = 0.0;
    for (size_t i = 0; i < list->count; i++)
        if (str_equal_ignore_case(list->items[i].product_name, product))
            total += list->items[i].transaction_amount;
    return total;
}

// operations chosen at runtime through function pointers
void op_total_revenue(const TransactionList *list) {
    printf("Total revenue: %.2f\n", calculate_total_revenue(list));
}

void op_average_value(const TransactionList *list) {
    if (list->count == 0) { printf("No transactions available.\n"); return; }
    printf("Average transaction value: %.2f\n", calculate_average_transaction_value(list));
}

void op_highest_transaction(const TransactionList *list) {
    const Transaction *t = find_highest_transaction(list);
    if (!t) { printf("No transactions available.\n"); return; }
    printf("Highest-value transaction:\n");
    print_header();
    print_transaction(t);
}

void op_lowest_transaction(const TransactionList *list) {
    const Transaction *t = find_lowest_transaction(list);
    if (!t) { printf("No transactions available.\n"); return; }
    printf("Lowest-value transaction:\n");
    print_header();
    print_transaction(t);
}

void op_total_units(const TransactionList *list) {
    printf("Total units sold: %ld\n", calculate_total_units_sold(list));
}

/* ---------- report ---------- */
void generate_sales_report(const TransactionList *list) {
    printf("\n========== SALES REPORT ==========\n");
    printf("Total transactions        : %zu\n", list->count);

    if (list->count == 0) {
        printf("No transaction data to report.\n");
        printf("==================================\n");
        return;
    }

    const Transaction *hi = find_highest_transaction(list);
    const Transaction *lo = find_lowest_transaction(list);

    printf("Total revenue             : %.2f\n", calculate_total_revenue(list));
    printf("Average transaction value : %.2f\n", calculate_average_transaction_value(list));
    printf("Highest transaction       : %.2f (ID %d, %s)\n", hi->transaction_amount, hi->transaction_id, hi->product_name);
    printf("Lowest transaction        : %.2f (ID %d, %s)\n", lo->transaction_amount, lo->transaction_id, lo->product_name);
    printf("Total quantity sold       : %ld\n", calculate_total_units_sold(list));
    printf("==================================\n");

    char product[128];
    if (read_line("Product for revenue breakdown (Enter to skip): ", product, sizeof product)
        && product[0] != '\0') {
        size_t matches = count_transactions(list, pred_product_equals, product);
        if (matches == 0)
            printf("No transactions found for product '%s'.\n", product);
        else
            printf("Revenue for '%s': %.2f (%zu transaction(s))\n",
                   product, calculate_product_revenue(list, product), matches);
    }
}