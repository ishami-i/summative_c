#include "main.h"

// helper
static int require_data(const TransactionList *list) {
    if (list->count == 0) { printf("No transactions recorded.\n"); return 0; }
    return 1;
}

// Returns choice in [min,max], or -1 if input was invalid.
static int read_menu_choice(const char *prompt, int min, int max) {
    int c, r = read_int(prompt, &c);
    if (r != 1 || c < min || c > max) {
        printf("Invalid selection. Enter a number from %d to %d.\n", min, max);
        return -1;
    }
    return c;
}

// tables of function pointers
typedef struct { const char *label; SalesOperation operation; } SalesOpEntry;
static const SalesOpEntry sales_ops[] = {
    { "Calculate total revenue",            op_total_revenue },
    { "Calculate average transaction value", op_average_value },
    { "Identify highest-value transaction", op_highest_transaction },
    { "Identify lowest-value transaction",  op_lowest_transaction },
    { "Calculate total units sold",         op_total_units },
};
#define SALES_OP_COUNT (sizeof sales_ops / sizeof sales_ops[0])

typedef struct { const char *label; TransactionComparator comparator; } SortEntry;
static const SortEntry sort_options[] = {
    { "Transaction ID",                cmp_by_id },
    { "Product name",                  cmp_by_product },
    { "Total amount (ascending)",      cmp_by_amount_asc },
    { "Total amount (descending)",     cmp_by_amount_desc },
};
#define SORT_COUNT (sizeof sort_options / sizeof sort_options[0])

// main-menu handlers
static void handle_add(TransactionList *l)     { add_transaction(l); }
static void handle_display(TransactionList *l) { display_transactions(l); }
static void handle_update(TransactionList *l)  { update_transaction(l); }
static void handle_delete(TransactionList *l)  { delete_transaction(l); }
static void handle_report(TransactionList *l)  { generate_sales_report(l); }
static void handle_save(TransactionList *l)    { save_transactions(l, DATA_FILE); }
static void handle_load(TransactionList *l)    { load_transactions(l, DATA_FILE); }

static void handle_process(TransactionList *l) {
    if (!require_data(l)) return;
    printf("\n--- Process Sales Data ---\n");
    for (size_t i = 0; i < SALES_OP_COUNT; i++) printf("%zu. %s\n", i + 1, sales_ops[i].label);
    printf("0. Back\n");

    int c = read_menu_choice("Choose operation: ", 0, (int)SALES_OP_COUNT);
    if (c <= 0) return;

    SalesOperation op = sales_ops[c - 1].operation;
    op(l);                                            
}

static void handle_filter(TransactionList *l) {
    if (!require_data(l)) return;
    printf("\n--- Filter Transactions (Callbacks) ---\n");
    printf("1. Show transactions above a sales amount\n");
    printf("2. Show transactions for a product\n");
    printf("3. Count transactions above a sales amount\n");
    printf("4. Count transactions with quantity at least N\n");
    printf("5. Count transactions for a product\n");
    printf("0. Back\n");

    int c = read_menu_choice("Choose: ", 0, 5);
    if (c <= 0) return;

    double amount; int qty; char product[PRODUCT_NAME_LEN];
    switch (c) {
    case 1: case 3:
        if (read_double("Enter amount: ", &amount) != 1) { printf("Invalid amount.\n"); return; }
        if (c == 1) filter_transactions(l, pred_amount_above, &amount);
        else printf("Number of transactions above %.2f: %zu\n", amount,
                    count_transactions(l, pred_amount_above, &amount));
        break;
    case 4:
        if (read_int("Enter minimum quantity: ", &qty) != 1) { printf("Invalid quantity.\n"); return; }
        printf("Number of transactions with quantity >= %d: %zu\n", qty,
               count_transactions(l, pred_quantity_at_least, &qty));
        break;
    case 2: case 5:
        if (read_string("Enter product name: ", product, sizeof product) != 1) {
            printf("Invalid product name.\n"); return;
        }
        if (c == 2) filter_transactions(l, pred_product_equals, product);
        else printf("Number of transactions for '%s': %zu\n", product,
                    count_transactions(l, pred_product_equals, product));
        break;
    }
}

static void handle_search(TransactionList *l) {
    if (!require_data(l)) return;
    printf("\n--- Search Transactions ---\n");
    printf("1. By Transaction ID\n2. By Product name\n3. By Transaction amount\n0. Back\n");
    int c = read_menu_choice("Choose: ", 0, 3);
    if (c == 1) search_by_id(l);
    else if (c == 2) search_by_product(l);
    else if (c == 3) search_by_amount(l);
}

static void handle_sort(TransactionList *l) {
    if (!require_data(l)) return;
    printf("\n--- Sort Transactions ---\n");
    for (size_t i = 0; i < SORT_COUNT; i++) printf("%zu. By %s\n", i + 1, sort_options[i].label);
    printf("0. Back\n");

    int c = read_menu_choice("Choose: ", 0, (int)SORT_COUNT);
    if (c <= 0) return;

    sort_transactions(l, sort_options[c - 1].comparator);
    printf("Sorted by %s.\n", sort_options[c - 1].label);
    display_transactions(l);
}

typedef struct { const char *label; MenuHandler handler; } MenuItem;
static const MenuItem main_menu[] = {
    { "Add New Transaction",           handle_add },
    { "Display All Transactions",      handle_display },
    { "Update Transaction",            handle_update },
    { "Delete Transaction",            handle_delete },
    { "Process Sales Data",            handle_process },
    { "Filter Transactions (Callbacks)", handle_filter },
    { "Search Transactions",           handle_search },
    { "Sort Transactions",             handle_sort },
    { "Generate Sales Report",         handle_report },
    { "Save Transactions",             handle_save },
    { "Load Transactions",             handle_load },
};
#define MENU_COUNT ((int)(sizeof main_menu / sizeof main_menu[0]))

int main(void) {
    TransactionList list;
    if (list_init(&list) != 0) return EXIT_FAILURE;

    load_transactions(&list, DATA_FILE);

    for (;;) {
        printf("\n===== SALES TRANSACTION MANAGEMENT SYSTEM =====\n");
        for (int i = 0; i < MENU_COUNT; i++) printf("%d. %s\n", i + 1, main_menu[i].label);
        printf("0. Exit\n");

        int c, r = read_int("Enter your choice: ", &c);
        if (r == -1) break;                      
        if (r == 0 || c < 0 || c > MENU_COUNT) {
            printf("Invalid selection. Enter a number from 0 to %d.\n", MENU_COUNT);
            continue;
        }
        if (c == 0) break;
        main_menu[c - 1].handler(&list);
    }

    save_transactions(&list, DATA_FILE);
    list_free(&list);
    printf("Exiting the program. Goodbye!\n");
    return EXIT_SUCCESS;
}