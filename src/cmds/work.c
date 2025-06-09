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
    {"id",      required_argument,      0, 'i'},  /* Name option */
    {0, 0, 0, 0}
};

static const char *work_short_options = "+hi:";

static const struct opt_fn work_option_fns[] = {
    {'h', work_help,    NULL},
    {'i', NULL,         work_on_item_id},
    {0, 0, 0}
};

void work_help() {
    printf("%s %s - work todo item for staging\n",
           CONF_NAME_UPPER,
           WORK_CMD_NAME);
    printf("usage: %s %s ...\n", CONF_CMD_NAME, WORK_CMD_NAME);
    printf("\n");
    printf("Documentation and usage code to be provided for %s\n",
           WORK_CMD_NAME);
}

void work_on_item_id(const char * id_str) {
    assert(id_str);

    sitem_id id = strtoll(id_str, NULL, 16);

    dir_change_item_status_id(id, IN_PROG);
}

int work_cmd(const int argc, char * const argv[], const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(argc, argv,
                                              work_short_options,
                                              work_long_options,
                                              work_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    return 0;
}
