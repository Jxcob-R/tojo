#include "depend.h"
#include "config.h"
#include "dir.h"
#include "ds/graph.h"
#include "ds/item.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option dep_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"add",     required_argument,      0, 'a'},
    {0, 0, 0, 0}
};

static const char *dep_short_options = "+ha:";

static const struct opt_fn dep_option_fns[] = {
    {'h', dep_help,     NULL},
    {'a', NULL,         dep_add_ids},
    {0, 0, 0}
};

void dep_help(void) {
    printf("%s %s - add a dependency between items\n",
           CONF_NAME_UPPER,
           DEP_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, DEP_CMD_NAME);
    printf("\n");
    printf("\t-h, --help\tBring up this help page\n");
    printf("\t-a, --add\tAdd a dependency between two tasks in the project\n");
    printf("\n");
}

/**
 * @brief Parse the list of new dependencies provided by the user
 * @param dep_str String of dependencies to be added
 * @param project_dependencies Dependencies already present in the project
 * @return List of dependencies provided by the user
 * @note Will free project_dependencies and set to NULL in the case that
 * parsing fails
 * @note Prints notes to stdout
 */
static struct dependency_list *
parse_dependencies_from_user(const char *dep_str,
                             struct dependency_list **project_dependencies) {
    if (!strchr(dep_str, *DEP_DELIM)) {
        printf("Depedencies not provided in the correct format\n");
        printf("Use <id-dependent>%s<id1>,<id2>,... to create dependencies",
               DEP_DELIM);
        return NULL;
    }

    /* Parse dependency string */
    strtok((char *) dep_str, DEP_DELIM);
    sitem_id from = strtoll(dep_str, NULL, 10);
    if (!dir_contains_item_with_id(from)) {
        printf("No item in project with ID %d\n", from);
        if (*project_dependencies) {
            graph_free_dependency_list(project_dependencies);
        }
        return NULL;
    }
    struct dependency_list *list = graph_init_dependency_list(0);
    char *to_str = strtok(NULL, DEP_SIBLING_DELIM);
    while (to_str) {
        sitem_id to = strtoll(to_str, NULL, 10);
        if (!dir_contains_item_with_id(to)) {
            printf("No item in project with ID %d\n", to);
            /* Continue anyway */
            continue;
        }
        struct dependency *dep = graph_new_dependency(from, to, 0);
        graph_new_dependency_to_list(list, &dep);
        to_str = strtok(NULL, DEP_SIBLING_DELIM);
    }

    return list;
}

void dep_add_ids(const char *dep_str) {
    struct dependency_list *project_dependencies = dir_get_all_dependencies();

    struct dependency_list *user_list =
        parse_dependencies_from_user(dep_str, &project_dependencies);

    if (!user_list) {
        printf("Could not add any dependencies between items\n");
        if (project_dependencies)
            graph_free_dependency_list(&project_dependencies);
        return;
    }

    user_list = graph_remove_duplicates(&user_list, project_dependencies);

    dir_add_dependency_list(user_list);

    graph_free_dependency_list(&user_list);
    if (project_dependencies)
        graph_free_dependency_list(&project_dependencies);
}

int dep_cmd(const int argc, char * const argv[],
               const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(argc, argv,
                                              dep_short_options,
                                              dep_long_options,
                                              dep_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }


    return 0;
}
