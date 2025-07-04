#include "config.h"
#include "dir.h"
#include "resolve.h"
#include "ds/item.h"
#include "opts.h"

#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/* Option names */
static const struct option res_long_options[] = {
    {"help",    no_argument,            0, 'h'}, /* Help option */
    {"id",      required_argument,      0, 'i'}, /* ID option */
    {"code",    required_argument,      0, 'c'}, /* Code option */
    {0, 0, 0, 0}
};

static const char *res_short_options = "+hi:c:";

static const struct opt_fn res_option_fns[] = {
    {'h', res_help,     NULL},
    {'i', NULL,         res_item_id},
    {'c', NULL,         res_item_code},
    {0, 0, 0}
};

void res_help() {
    printf("%s %s - fin todo item for staging\n",
           CONF_NAME_UPPER,
           RES_CMD_NAME);
    printf("usage: %s %s [<options>]\n", CONF_CMD_NAME, RES_CMD_NAME);
    printf("\n");
    printf("\t-i, --id\tResolve the item with the given id\n");
    printf("\t-c, --code\tRestage an already existing item by its code\n");
    printf("\t-h, --help\tBring up this help page\n");
}

void res_item_id(const char *id_str) {
    assert(id_str);

    sitem_id id = strtoll(id_str, NULL, 10);

    dir_change_item_status_id(id, DONE);
}

void res_item_code(const char *code) {
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
        dir_change_item_status_id(id, DONE);
    }
}

int res_cmd(const int argc, char * const *argv, const char *proj_path) {
    assert(proj_path);

    if (*proj_path == '\0') {
        printf("Not in a project\n");
        return RET_NO_PROJ;
    }

    const int opts_handled = opts_handle_opts(argc, argv,
                                              res_short_options,
                                              res_long_options,
                                              res_option_fns);

    if (opts_handled < 0) {
        printf("Unknown options provided");
        return RET_INVALID_OPTS;
    }

    return 0;
}
