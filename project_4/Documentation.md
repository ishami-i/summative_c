# Sales Transaction Management System (C)

## How to run
    bash run.sh        # or: ./run.sh   (after chmod +x run.sh once)
`run.sh` runs `chmod +x` on every file, compiles all `.c` files with `gcc -Wall -Wextra -std=c11`, then starts `sales_app`.
`transactions.txt` already contains 8 sample records; they are loaded at startup.

## File structure
| File | Responsibility |
|---|---|
| `main.h` | Structs, function-pointer typedefs, all prototypes |
| `main.c` | Menu tables (function-pointer dispatch), sub-menus, startup load / exit save |
| `transaction_management.c` | Dynamic array (malloc/realloc/free), add / display / update / delete, validation |
| `transactions_processing.c` | Calculations, the 5 sales operations, sales report |
| `callback_functions.c` | Predicate callbacks + `filter_transactions` / `count_transactions` |
| `search_sort.c` | Comparator callbacks, manual insertion sort, linear searches |
| `file_handling.c` | Save / load (text format `id|product|qty|price|total|date`) |
| `utils.c` | Safe input (fgets + strtol/strtod), date check, case-insensitive compare, printing |
| `sample_run.txt` | Captured output of two sessions (evidence) |

## How the required C concepts are implemented
**Function pointers** - `main.c` holds a table `main_menu[]` of `MenuHandler` pointers (the main menu calls `main_menu[c-1].handler(&list)`), and a table `sales_ops[]` of `SalesOperation` pointers. In *Process Sales Data* the user picks an operation at runtime and it is executed with `op(l)`. `sort_options[]` is a third table, of comparators.

**Callbacks** - `filter_transactions(list, pred, ctx)` and `count_transactions(list, pred, ctx)` receive a `TransactionPredicate` as an argument and call `pred(&item, ctx)` for each record. Callbacks: `pred_amount_above`, `pred_product_equals`, `pred_quantity_at_least`, `pred_amount_equals`. `sort_transactions(list, cmp)` receives a `TransactionComparator` (`cmp_by_id`, `cmp_by_product`, `cmp_by_amount_asc`, `cmp_by_amount_desc`) and invokes it to compare records.

**Dynamic memory** - `list_init` uses `malloc` (capacity 4). `list_ensure_capacity` uses `realloc` (doubling) through a temporary pointer so the old block is not lost on failure. `list_free` calls `free`. Every allocation failure is checked and reported. Records are accessed through pointers (`Transaction *items`).

**Search / sort** - linear searches by ID, product, amount. Insertion sort written by hand (no `qsort`), stable.

**Validation** - duplicate/non-positive IDs, non-positive quantity or price, non-numeric input, bad dates, invalid menu choices, empty data, missing/invalid IDs, average with zero records (returns 0, no division), file open/read/write/close errors, malformed or duplicate lines in the data file (skipped with a warning).

**File handling** - records are loaded at startup, saved on exit (and via menu option 10), so they persist between runs. Load replaces current data only if the whole file was read successfully.