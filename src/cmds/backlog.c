#include "backlog.h"
#include "config.h"
#include "dir.h"
#include "ds/item.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option back_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"id",      required_argument,      0, 'i'}, /* ID backlog option */
    {"code",    required_argument,      0, 'c'}, /* Code option */
    {0, 0, 0, 0}
};

static const char *back_short_options = "+hi:c:";

static const struct opt_fn back_option_fns[] = {
    {'h', back_help,    NULL},
    {'i', NULL,         back_item_id},
    {'c', NULL,         back_item_code},
    {0, 0, 0}
};

void back_help() {
    printf("%s %s - place item in 'backlog'\n",
           CONF_NAME_UPPER,
           BACK_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, BACK_CMD_NAME);
    printf("\n");
    printf("\t-h, --help\tBring up this help page\n");
    printf("\t-i, --id\tBacklog the item with some given ID\n");
    printf("\t-c, --code\tBacklog the item with some given item code\n");
    printf("\n");
    printf("usage: %s %s <code>\n", CONF_CMD_NAME, BACK_CMD_NAME);
    printf("\n");
    printf("Backlog item with the code (same as using -c/--code)");
}

void back_item_id(const char *id_str) {
    assert(id_str);

    sitem_id id = strtoll(id_str, NULL, 10);

    if (dir_change_item_status_id(id, BACKLOG) == 0)
        printf("Backlogged item with ID: %s\n", id_str);
    else
        printf("Incorrect ID for item provided\n");
}

void back_item_code(const char *code) {
    /* This function can be called by default */
    assert(code);

    sitem_id id = -1;
    if (!item_is_valid_code(code)) {
        puts("Please provide a valid code or code prefix");
        return;
    }

    if (strlen(code) == ITEM_CODE_LEN) {
        item *itp = dir_get_item_with_code(code);
        if (!itp) {
            puts("Invalid code provided");
            return;
        }
        id = itp->item_id;
        item_free(itp);
    } else {
        id = dir_get_id_from_prefix(code);
    }
    if (id < 0) {
        printf("No item found with code %s\n", code);
    } else {
        if (dir_change_item_status_id(id, BACKLOG) == 0)
            printf("Backlogging item with ID: %d\n", id);
        else
            printf("Item is already in 'backlog'\n");
    }
}


int back_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(
        argc, argv,
                                              back_short_options,
                                              back_long_options,
                                              back_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    if (opts_handled == 0 && argc > 1) {
        back_item_code(argv[1]);
    }

    return 0;
}
