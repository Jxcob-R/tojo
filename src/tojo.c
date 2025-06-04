#include "config.h"
#include "opts.h"
#include "tojo.h"

#include "cmds/add.h"
#include "cmds/init.h"
#include "cmds/list.h"
#include "cmds/remove.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option tj_long_options[] = {
    {"help",    no_argument,    0, 'h'}, /* Help option */
    {"version", no_argument,    0, 'v'}, /* Version option */
    {0, 0, 0, 0}
};

static const char *tj_short_options = "+hv";

static const struct opt_fn tj_option_fns[] = {
    {'h', tj_help,          NULL},
    {'v', tj_print_vers,    NULL},
    {0, 0, 0}
};

/* Commands */

static const struct cmd tj_cmds[] = {
    {INIT_CMD_NAME, init_cmd},     /* Project initialisation */
    {LIST_CMD_NAME, list_cmd},     /* List items */
    {ADD_CMD_NAME, add_cmd},       /* Add an item */
    {NULL, NULL}
};

static const struct cmd * get_cmd(char *name) {
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
    printf("usage: %s ...\n", CONF_CMD_NAME);
    printf("\n");
    printf("Documentation and usage code to be provided\n");
}

void tj_print_vers() {
    printf("%s version: %s\n", CONF_CMD_NAME, CONF_VERSION);
    printf("\n");
    printf("For more versions go to %s\n", CONF_GITHUB);
}

/*
 * @brief Get the user's home directory
 */
static char* get_home_directory(void) {
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_dir : NULL;
}

/*
 * @brief Check if a directory exists and is accessible
 */
static int is_accessible_directory(const char *path) {
    if (access(path, F_OK | R_OK | W_OK) != 0) {
        return 0;
    }
    
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    
    return S_ISDIR(st.st_mode);
}

/*
 * @brief Move up one directory level in the path
 */
static int move_up_directory(char *path) {
    char *last_slash = strrchr(path, '/');
    
    if (last_slash == NULL || last_slash == path) {
        return -1;  /* Can't go up further */
    }
    
    *last_slash = '\0';
    return 0;
}

/*
 * @brief Build relative path with appropriate number of "../" prefixes
 */
static void build_relative_path(char *dest, int levels_up,
                                const char *target_dir) {
    dest[0] = '\0';
    
    if (levels_up == 0) {
        strcpy(dest, target_dir);
        return;
    }
    
    for (int i = 0; i < levels_up; i++) {
        if (i > 0) strcat(dest, "/");
        strcat(dest, "..");
    }
    strcat(dest, "/");
    strcat(dest, target_dir);
}

/*
 * @brief Search for target directory starting from current path, moving up
 * until home_dir
 */
static int find_target_directory(const char *start_path, const char *home_dir, 
                                 const char *target_dir, int *levels_up) {
    char search_path[_MAX_PATH];
    char test_path[_MAX_PATH + sizeof(CONF_PROJ_DIR)];
    
    strcpy(search_path, start_path);
    *levels_up = 0;
    
    while (*levels_up < _MAX_PATH_LVLS) {
        /* Check if we've reached the home directory (exclusive) */
        if (strcmp(search_path, home_dir) == 0) {
            return -1;
        }
        
        /* Construct path to test */
        snprintf(test_path, sizeof(test_path), "%s/%s", search_path,
                 target_dir);
        
        /* Test if directory exists and is accessible */
        if (is_accessible_directory(test_path)) {
            return 0;  /* Found it! */
        }
        
        /* Move up one directory level */
        if (move_up_directory(search_path) != 0) {
            return -1;  /* Can't go up further */
        }
        
        (*levels_up)++;
    }
    
    return -1;  /* Not found or too many levels */
}

int tj_get_proj_dir(char *dir) {
    assert(dir);
    
    char cwd[_MAX_PATH];
    char *home_dir;
    int levels_up;
    
    /* Get current working directory */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return -1;
    }
    
    /* Get home directory */
    home_dir = get_home_directory();
    if (home_dir == NULL) {
        return -1;
    }
    
    /* Search for the target directory */

    if (find_target_directory(cwd, home_dir, CONF_PROJ_DIR, &levels_up) == 0) {
        /* Project has been found */
        /* Build the relative path */
        build_relative_path(dir, levels_up, CONF_PROJ_DIR);

        return 0;
    }
    
    /* No project found */
    dir[0] = '\0';
    return -1;
}

int tj_main(const int argc, char * const argv[]) {
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
    char proj_dir[_MAX_PATH];
    if (tj_get_proj_dir(proj_dir) != 0) {
#ifdef DEBUG
        log_err("Not inside a project");
#endif
    }

    /* Pass control to sub-module with modified argc and argv */
    return subcommand->cmd_fn(argc - 1, argv + 1, proj_dir);
}

