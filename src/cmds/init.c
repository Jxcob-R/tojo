#include "init.h"
#include "config.h"
#include "dir.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option init_long_options[] = {
    {"help", no_argument, 0, 'h'}, /* Help option */
    {0, 0, 0, 0}};

static const char *init_short_options = "+h";

static const struct opt_fn init_option_fns[] = {{'h', init_help, NULL},
                                                {0, 0, 0}};

void init_help() {
    printf("%s %s - Initialise a project at the current directory\n",
           CONF_NAME_UPPER, INIT_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, INIT_CMD_NAME);
    printf("\n");
    printf("\t-h, --help\tBring up this help page\n");
}

int init_create_project() {
    /* Create project at CWD */

    if (dir_init(CONF_PROJ_DIR) == -1) {
#ifdef DEBUG
        log_err("Project could not be created at desired location:");
        log_err(CONF_PROJ_DIR);
#endif
        puts("Project already exists");
        return -1;
    }

    printf("Project successfully created\n");
    return 0;
}

int init_cmd(const int argc, char *const argv[], const char *proj_path) {
    assert(proj_path);

    const int opts_handled = opts_handle_opts(
        argc, argv, init_short_options, init_long_options, init_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided\n");
        return RET_INVALID_OPTS;
    }

    /* Check project existence */
    if (strlen(proj_path) >= sizeof(CONF_PROJ_DIR)) {
        /* Project directory exists */
        printf("Already inside a %s project, with directory located at %s\n",
               CONF_CMD_NAME, proj_path);
#ifdef DEBUG
        log_err("Project directory found");
#endif
        return RET_INIT_TJ_EXISTS;
    }

    /*
     * Checking opts_handled is a workable solution to prevent spurious init
     * attempts for now
     */
    if (opts_handled == 0 && init_create_project() == -1) {
        return RET_UNABLE_TO_INIT;
    }

    return 0;
}
