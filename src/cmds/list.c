#include "list.h"
#include "dir.h"
#include "ds/item.h"
#include "opts.h"
#include "config.h"
#include "ds/trie.h"

/* Option names */
static const struct option list_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"all",     no_argument,            0, 'a'}, /* List all task items */
    {0, 0, 0, 0}
};

static const char *list_short_options = "+ha";

static const struct opt_fn list_option_fns[] = {
    {'h', list_help,        NULL},
    {'a', list_all_names,   NULL},
    {0, 0, 0}
};

const char *path;

int * list_item_code_prefixes(item *const *items) {
    /* Yes, I've assigned a macro to a variable, its because I'm paranoid */
    const unsigned int code_len = ITEM_CODE_LEN;
    unsigned int num_items;
    for (num_items = 0; items[num_items]; num_items++);

    /* Make char list from item codes */
    const char **codes = (const char **)
                                  malloc(sizeof(char *) * num_items);

    for (unsigned int i = 0; i < num_items; i++)
        /* Create shallow copy in codes array */
        codes[i] = items[i]->item_code;

    /* Array to return */
    int *code_prefix_lengths = (int *) malloc(sizeof(int) * num_items);
    if (!code_prefix_lengths) return code_prefix_lengths;

    shortest_unique_prefix_lengths(codes, num_items, code_len, ITEM_CODE_CHARS,
                                   code_prefix_lengths);
    free(codes);
    return code_prefix_lengths;
}

void list_help() { printf("%s %s - list items in project\n",
           CONF_NAME_UPPER,
           LIST_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, LIST_CMD_NAME);
    printf("\n");
    printf("\t-a, --all\tList all current tasks in project\n");
    printf("\t-h, --help\tBring up this help page\n");
}

void list_all_names() {
    item **list_items = dir_read_all_items();

    printf("Current tasks open in this project:\n");

    /* Traverse array of items */
    unsigned int curr_item = 0;

    /* Get the prefixes of the item codes to show in list */
    int *item_code_prefix_lengths = list_item_code_prefixes(list_items);

    dir_write_item_codes(list_items, item_code_prefix_lengths);

    /* Print out items */
    while (list_items[curr_item] != NULL) {
        assert(item_code_prefix_lengths[curr_item] > 0);
        item_print_fancy(list_items[curr_item],
                         ITEM_PRINT_ID | ITEM_PRINT_NAME | ITEM_PRINT_CODE,
                         &item_code_prefix_lengths[curr_item]);
        item_free(list_items[curr_item]);
        curr_item++;
    }

    free(list_items);
    free(item_code_prefix_lengths);
}

int list_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    /* Set global variable */
    path = proj_path;

    const int opts_handled = opts_handle_opts(argc, argv,
                                              list_short_options,
                                              list_long_options,
                                              list_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    if (opts_handled == 0) {
        /* Default behaviour should be -a */
        list_all_names();
    }

    return 0;
}
