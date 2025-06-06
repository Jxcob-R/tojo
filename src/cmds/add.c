#include "add.h"
#include "config.h"
#include "dir.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option add_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"name",    required_argument,      0, 'n'}, /* Name option */
    {0, 0, 0, 0}
};

static const char *add_short_options = "+hn:";

static const struct opt_fn add_option_fns[] = {
    {'h', add_help,     NULL},
    {'n', NULL,         add_item_name},
    {0, 0, 0}
};

/**
 * Item to add on command execution
 * Modified by appropriate options and written to project at conclusion of
 * execution of command
 */
item it = {-1, (char[CONF_ITEM_NAME_SZ]) {"\0"}, TODO};

void add_help() {
    printf("%s %s - add todo item for staging\n",
           CONF_NAME_UPPER,
           ADD_CMD_NAME);
    printf("usage: %s %s ...\n", CONF_CMD_NAME, ADD_CMD_NAME);
    printf("\n");
    printf("Documentation and usage code to be provided for %s\n",
           ADD_CMD_NAME);
}

void add_item_name(const char *name) {
    assert(name);
    item_set_name_deep(&it, name);
    printf("Added item '%s' to task list for project\n", name);
}

int add_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project");
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
    if (dir_write_item(&it, proj_path) == -1)
        return -1;

    return 0;
}
