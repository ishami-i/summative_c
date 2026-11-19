#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define PRODUCT_NAME_LEN 50
#define DATE_LEN 11
#define INITIAL_CAPACITY 4
#define DATA_FILE "transactions.txt"

// data structures 
typedef struct {
    int    transaction_id;
    char   product_name[PRODUCT_NAME_LEN];
    int    quantity;
    double unit_price;
    double transaction_amount;
    char   transaction_date[DATE_LEN];
} Transaction;

// Dynamic array of transactions
typedef struct {
    Transaction *items;
    size_t count;
    size_t capacity;
} TransactionList;

// Function pointer types
typedef void (*MenuHandler)(TransactionList *);         
typedef void (*SalesOperation)(const TransactionList *);      
typedef int  (*TransactionPredicate)(const Transaction *, const void *ctx);  
typedef int  (*TransactionComparator)(const Transaction *, const Transaction *); 

// utils.c
int  read_line(const char *prompt, char *buf, size_t size);      
int  read_int(const char *prompt, int *out);                   
int  read_double(const char *prompt, double *out);              
int  read_string(const char *prompt, char *out, size_t size);    
void get_today(char *buf, size_t size);
int  is_valid_date(const char *s);
int  str_equal_ignore_case(const char *a, const char *b);
int  compare_ignore_case(const char *a, const char *b);
void print_header(void);
void print_transaction(const Transaction *t);

// transaction_management.c
int  list_init(TransactionList *list);
int  list_ensure_capacity(TransactionList *list);
int  list_append(TransactionList *list, const Transaction *t);
void list_free(TransactionList *list);
int  find_index_by_id(const TransactionList *list, int id);
int  add_transaction(TransactionList *list);
void display_transactions(const TransactionList *list);
int  update_transaction(TransactionList *list);
int  delete_transaction(TransactionList *list);

// transactions_processing.c 
double calculate_total_revenue(const TransactionList *list);
double calculate_average_transaction_value(const TransactionList *list);
const Transaction *find_highest_transaction(const TransactionList *list);
const Transaction *find_lowest_transaction(const TransactionList *list);
long   calculate_total_units_sold(const TransactionList *list);
double calculate_product_revenue(const TransactionList *list, const char *product);
// operations selectable through function pointers
void op_total_revenue(const TransactionList *list);
void op_average_value(const TransactionList *list);
void op_highest_transaction(const TransactionList *list);
void op_lowest_transaction(const TransactionList *list);
void op_total_units(const TransactionList *list);
void generate_sales_report(const TransactionList *list);

// callback_functions.c
int    pred_amount_above(const Transaction *t, const void *ctx);   /* ctx: const double *   */
int    pred_amount_equals(const Transaction *t, const void *ctx);  /* ctx: const double *   */
int    pred_product_equals(const Transaction *t, const void *ctx); /* ctx: const char *     */
int    pred_quantity_at_least(const Transaction *t, const void *ctx); /* ctx: const int *   */
size_t filter_transactions(const TransactionList *list, TransactionPredicate pred, const void *ctx);
size_t count_transactions(const TransactionList *list, TransactionPredicate pred, const void *ctx);

// search_sort.c
int  cmp_by_id(const Transaction *a, const Transaction *b);
int  cmp_by_product(const Transaction *a, const Transaction *b);
int  cmp_by_amount_asc(const Transaction *a, const Transaction *b);
int  cmp_by_amount_desc(const Transaction *a, const Transaction *b);
void sort_transactions(TransactionList *list, TransactionComparator cmp);
void search_by_id(const TransactionList *list);
void search_by_product(const TransactionList *list);
void search_by_amount(const TransactionList *list);

// file_handling.c
int save_transactions(const TransactionList *list, const char *filename);
int load_transactions(TransactionList *list, const char *filename);

#endif /* MAIN_H */