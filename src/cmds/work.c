#include "work.h"
#include "config.h"
#include "dir.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option work_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"id",      required_argument,      0, 'i'}, /* Name option */
    {"code",    required_argument,      0, 'c'}, /* Code option */
    {0, 0, 0, 0}
};

static const char *work_short_options = "+hi:c:";

static const struct opt_fn work_option_fns[] = {
    {'h', work_help,    NULL},
    {'i', NULL,         work_on_item_id},
    {'c', NULL,         work_on_item_code},
    {0, 0, 0}
};

void work_help() {
    printf("%s %s - work todo item for staging\n",
           CONF_NAME_UPPER,
           WORK_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, WORK_CMD_NAME);
    printf("\n");
    printf("\t-i, --id\tMove item with specified ID to in progress; "
                        "item may have any state\n");
    printf("\t-c, --code\tWork on an item with the given code\n");
    printf("\t-h, --help\tBring up this help page\n");
    printf("\n");
    printf("usage: %s %s [<code>]\n", CONF_CMD_NAME, WORK_CMD_NAME);
    printf("\n");
    printf(
        "Using an established item code (or prefix) marks item as in-progress\n"
    );
}

void work_on_item_id(const char *id_str) {
    assert(id_str);

    sitem_id id = strtoll(id_str, NULL, 10);

    if (dir_change_item_status_id(id, IN_PROG) == 0)
        printf("Marked item with ID: %s as 'in-progress'\n", id_str);
}

void work_on_item_code(const char *code) {
    assert(code);

    sitem_id id = -1;
    if (strlen(code) == ITEM_CODE_LEN) {
        item *itp = dir_get_item_with_code(code);
        if (!itp) {
            printf("Invalid code provided");
            return;
        }
        id = itp->item_id;
        item_free(itp);
    } else if (strlen(code) > 0) {
        id = dir_get_id_from_prefix(code);
    } else {
        return;
    }
    if (id < 0) {
        printf("No item found with code %s\n", code);
    } else {
        if (dir_change_item_status_id(id, IN_PROG) == 0)
            printf("Marked item with ID: %d as 'in-progress'\n", id);
        else
            printf("Item is already 'in-progress'\n");
    }
}

int work_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(argc, argv,
                                              work_short_options,
                                              work_long_options,
                                              work_option_fns);

    /* Using item code is default behaviour (i.e. using -c) */
    char *const arg = argv[1];
    if (opts_handled == 0 && arg) {
        work_on_item_code(arg);
    }

    return 0;
}
