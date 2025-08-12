#include "tojo.h"
#include "config.h"
#include "dir.h"
#include "opts.h"

#include "cmds/add.h"
#include "cmds/backlog.h"
#include "cmds/depend.h"
#include "cmds/init.h"
#include "cmds/list.h"
#include "cmds/resolve.h"
#include "cmds/work.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option tj_long_options[] = {
    {"help", no_argument, 0, 'h'},    /* Help option */
    {"version", no_argument, 0, 'v'}, /* Version option */
    {0, 0, 0, 0}};

static const char *tj_short_options = "+hv";

static const struct opt_fn tj_option_fns[] = {
    {'h', tj_help, NULL}, {'v', tj_print_vers, NULL}, {0, 0, 0}};

/* Commands */

static const struct cmd tj_cmds[] = {
    {ADD_CMD_NAME, add_cmd},   /* Add an item */
    {BACK_CMD_NAME, back_cmd}, /* Backlog an item */
    {DEP_CMD_NAME, dep_cmd},   /* Add dependency between items */
    {INIT_CMD_NAME, init_cmd}, /* Project initialisation */
    {LIST_CMD_NAME, list_cmd}, /* List items */
    {WORK_CMD_NAME, work_cmd}, /* Commence work on an item */
    {RES_CMD_NAME, res_cmd},   /* Commence work on an item */
    {NULL, NULL}};

static const struct cmd *get_cmd(char *name) {
    const struct cmd *target = NULL;

    for (unsigned int i = 0; tj_cmds[i].cmd_name != NULL; i++) {
        if (strcmp(tj_cmds[i].cmd_name, name) == 0) {
            target = tj_cmds + i;
            break;
        }
    }

    return target;
}

void tj_help() {
    printf("%s - Terminal TOdo JOtter:\n", CONF_NAME_UPPER);
    printf("A CLI to-do tool that can help you track project progress"
           "locally");
    printf("usage: %s [<options>]\n", CONF_CMD_NAME);
    printf("\n");
    printf("\t-h, --help\tBring up this help page\n");
    printf("\n");
    printf("usage: %s <command>\n", CONF_CMD_NAME);
    printf("\tinit\tInitialise project\n");
    printf("\tadd\tAdd items to project\n");
    printf("\tres\tResolve open items\n");
    printf("\twork\tMark items as in-progress\n");
    printf("\tlist\tList items in project\n");
    printf("\n");
    printf("See more details of each command in individual help pages\n");
}

void tj_print_vers() {
    printf("%s version: %s\n", CONF_CMD_NAME, CONF_VERSION);
    printf("\n");
    printf("For more versions go to %s\n", CONF_GITHUB);
}

int tj_main(const int argc, char *const argv[]) {
    assert(argv);

    const int opts_handled = opts_handle_opts(argc, argv, tj_short_options,
                                              tj_long_options, tj_option_fns);

    if (opts_handled < 0) {
        return RET_INVALID_OPTS;
    }

    const struct cmd *subcommand;

    if (opts_handled == 0) {
        /* Find appropriate sub-module */
        assert(argv[1]);
        subcommand = get_cmd(argv[1]);

        if (!subcommand) {
            printf("'%s' is not a command. See help page\n", argv[1]);
            return RET_INVALID_CMD;
        }
    } else {
        /* Options were handled */
        return 0;
    }

    /* Find TJ project directory */
    char proj_dir[MAX_PATH];
    if (dir_find_project(proj_dir) != 0) {
#ifdef DEBUG
        log_err("Not inside a project");
#endif
    }

    /* Pass control to sub-module with modified argc and argv */
    return subcommand->cmd_fn(argc - 1, argv + 1, proj_dir);
}
