#include "list.h"
#include "config.h"
#include "dir.h"
#include "ds/graph.h"
#include "ds/item.h"
#include "ds/trie.h"
#include "opts.h"

/* Option names */
static const struct option list_long_options[] = {
    {"help", no_argument, 0, 'h'},               /* Help option */
    {"all", no_argument, 0, 'a'},                /* List all task items */
    {"status", required_argument, 0, 's'},       /* List all task items */
    {"dependencies", required_argument, 0, 'd'}, /* List item dependencies */
    {0, 0, 0, 0}};

static const char *list_short_options = "+has:d:";

static const struct opt_fn list_option_fns[] = {{'h', list_help, NULL},
                                                {'a', list_all_names, NULL},
                                                {'s', NULL, list_by_status},
                                                {'d', NULL, list_dependencies},
                                                {0, 0, 0}};

int *list_item_code_prefixes(item *const *items) {
    /* Yes, I've assigned a macro to a variable, its because I'm paranoid */
    const unsigned int code_len = ITEM_CODE_LEN;
    unsigned int num_items;
    for (num_items = 0; items[num_items]; num_items++)
        ;

    /* Make char list from item codes */
    const char **codes = (const char **)malloc(sizeof(char *) * num_items);

    for (unsigned int i = 0; i < num_items; i++)
        /* Create shallow copy in codes array */
        codes[i] = items[i]->item_code;

    /* Array to return */
    int *code_prefix_lengths = (int *)malloc(sizeof(int) * num_items);
    if (!code_prefix_lengths)
        return code_prefix_lengths;

    shortest_unique_prefix_lengths(codes, num_items, code_len, ITEM_CODE_CHARS,
                                   code_prefix_lengths);
    free(codes);
    return code_prefix_lengths;
}

void list_help() {
    printf("%s %s - list items in project\n", CONF_NAME_UPPER, LIST_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, LIST_CMD_NAME);
    printf("\n");
    printf("\t-a, --all\tList all current tasks in project\n");
    printf("\t-h, --help\tBring up this help page\n");
}

/**
 * @brief Print all items in array of item pointers with item code and other
 * given flags
 * @param items Array of item pointers
 * @param item_print_flags
 * @see item_print_fancy for flag options
 * @note Frees all items as well, thus, it is assumed that items is a
 * heap-allocated array
 */
static void print_list_items_codes(item **items, long long item_print_flags) {
    /* Traverse array of items */
    unsigned int curr_item = 0;

    /* Get the prefixes of the item codes to show in list */
    int *item_code_prefix_lengths = list_item_code_prefixes(items);

    dir_write_item_codes(items, item_code_prefix_lengths);

    /* Print out items */
    while (items[curr_item] != NULL) {
        assert(item_code_prefix_lengths[curr_item] > 0);
        item_print_fancy(items[curr_item], item_print_flags | ITEM_PRINT_CODE,
                         &item_code_prefix_lengths[curr_item]);
        item_free(items[curr_item]);
        curr_item++;
    }

    free(items);
    free(item_code_prefix_lengths);
}

void list_all_names() {
    item **list_items = dir_read_all_items();

    printf("Current tasks open in this project:\n");

    print_list_items_codes(list_items, ITEM_PRINT_ID | ITEM_PRINT_NAME);
}

/**
 * @brief Find any duplicate status characters in the provided status string
 * @param status_str Status string (e.g. "tid" == "todo + in-prog + done" items)
 * @param len Number of characters to check for duplicates
 * @return Encoded sequence of duplicates (1 = duplicate in position; 0 = fine)
 * The sequence is encoded from the *least* significant end
 * Example:
 * 0...01010 => indices 1 and 3 in status_str are duplicates
 * @note the least significant bit is always 0
 * @see list_by_status for status_str expectations
 */
static int64_t get_dup_status_chars(const char *const status_str,
                                    const uint8_t len) {
    assert(len <= 8 * sizeof(int64_t));

    int64_t duplicate_mask = 0;
    for (int i = 1; i < len; i++) {
        for (int j = 0; j < i; j++) {
            duplicate_mask |= ((status_str[i] == status_str[j]) << i);
        }
    }
    return duplicate_mask;
}

void list_by_status(const char *status_str) {
    assert(status_str);

    size_t chars_in_status_str = strlen(status_str);

    /* Set a reasonable capacity */
    unsigned int list_capacity = 64;
    unsigned int num_items = 0;
    item **list_items = item_array_init_empty(list_capacity);

    if (!list_items)
        return;

    uint64_t duplicate_mask =
        get_dup_status_chars(status_str, ITEM_STATUS_COUNT);

    /*
     * Since these are processed in order, this gives the user the ability to
     * customise the order of output.
     * Note that re-printing the same status is avoided
     */
    for (size_t i = 0; i < chars_in_status_str && i < ITEM_STATUS_COUNT; i++) {
        if ((duplicate_mask >> i) & 1)
            continue; /* Duplicate */

        item **status_items = NULL;
        switch (status_str[i]) {
        case LIST_BACKLOG_CHAR:
            status_items = dir_read_items_status(BACKLOG);
            break;
        case LIST_TODO_CHAR:
            status_items = dir_read_items_status(TODO);
            break;
        case LIST_IP_CHAR:
            status_items = dir_read_items_status(IN_PROG);
            break;
        case LIST_DONE_CHAR:
            status_items = dir_read_items_status(DONE);
            break;
        default:
            break; /* Character not expected */
        }
        if (status_items) {
            /* There are a couple of potential unsafe operations here */
            unsigned int new_num_items = item_count_items(status_items);
            if (num_items + new_num_items > list_capacity) {
                list_capacity *= 2;
                list_items = item_array_resize(list_items, list_capacity);
            }

            if (!list_items)
                return;

            item_array_add(list_items + num_items, status_items, SIZE_MAX);
            num_items += new_num_items;

            free(status_items);
        }
    }

    print_list_items_codes(list_items, ITEM_PRINT_ID | ITEM_PRINT_NAME);

    if (strlen(status_str) > ITEM_STATUS_COUNT) {
        puts("\nOnly the first three specified statuses where listed");
    }
}

void list_dependencies(const char *id_str) {
    sitem_id id = strtoll(id_str, NULL, 10);
    if (!dir_contains_item_with_id(id)) {
        printf("Project does not contain item %d\n", id);
        return;
    }
    item **project_items = dir_read_all_items();
    struct dependency_list *project_dependencies = dir_get_all_dependencies();

    struct graph_of_items *full_proj_dag =
        graph_create_graph(&project_items, &project_dependencies);

    /* Obtain only relevant part of dependency graph */
    struct graph_of_items *target_dag =
        graph_get_subgraph_to_item(&full_proj_dag, id);

    /* No item codes listed */
    graph_print_dag_with_item_fields(target_dag, id,
                                     ITEM_PRINT_ID | ITEM_PRINT_NAME);

    graph_free_graph(&target_dag);
}

int list_cmd(const int argc, char *const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(
        argc, argv, list_short_options, list_long_options, list_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    if (opts_handled == 0) {
        if (argc == 1)
            /* Ignore backlog by default -- see list_by_status */
            list_by_status((const char[]){LIST_TODO_CHAR, LIST_IP_CHAR,
                                          LIST_DONE_CHAR, 0});
        else
            list_by_status(argv[1]);
    }

    return 0;
}
