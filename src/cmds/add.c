#include "add.h"
#include "config.h"
#include "dir.h"
#include "ds/item.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option add_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"name",    required_argument,      0, 'n'}, /* Name option */
    {"code",    required_argument,      0, 'c'}, /* Code option */
    {"restage", required_argument,      0, 'r'}, /* ID re-stage option */
    {0, 0, 0, 0}
};

static const char *add_short_options = "+hr:c:n:";

static const struct opt_fn add_option_fns[] = {
    {'h', add_help,     NULL},
    {'r', NULL,         add_restage_item_id},
    {'c', NULL,         add_restage_item_code},
    {'n', NULL,         add_item_name},
    {0, 0, 0}
};

/**
 * Item to add on command execution
 * Modified by appropriate options and written to project at conclusion of
 * execution of command
 */
static item it = {
    .item_id = -1, 
    .item_code = { "z" },
    .item_name = (char[ITEM_NAME_MAX]) {"\0"},
    .item_st = TODO
};

void add_help() {
    printf("%s %s - add todo item to project\n",
           CONF_NAME_UPPER,
           ADD_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, ADD_CMD_NAME);
    printf("\n");
    printf("\t-n, --name\tAdd item by name\n");
    printf("\t-r, --restage\tRestage an already existing item by its item ID\
            \n");
    printf("\t-c, --code\tRestage an already existing item by its code \n");
    printf("\t-h, --help\tBring up this help page\n");
}

void add_restage_item_id(const char *id_str) {
    assert(id_str);

    sitem_id id = strtoll(id_str, NULL, 10);

    dir_change_item_status_id(id, TODO);
}

void add_restage_item_code(const char *code) {
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
        dir_change_item_status_id(id, TODO);
    }
}

void add_item_name(const char *name) {
    assert(name);
    item_set_name_deep(&it, name, strlen(name) + 1);
    printf("Added item '%s' to task list for project\n", name);

    /* ID set to next available number */
    it.item_id = dir_next_id();
    item_set_code(&it);
}

int add_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(argc, argv,
                                              add_short_options,
                                              add_long_options,
                                              add_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    /* Let name-based task addition be the default */
    if (opts_handled == 0 && argv[1]) {
        add_item_name(argv[1]);
    }

    /* Write added item to appropriate location */
    if (it.item_id != -1 && dir_append_item(&it) == -1)
        return -1;

    return 0;
}
