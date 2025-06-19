#include "item.h"

item * item_init() {
    item *itp = (item *) malloc(sizeof(item));

    itp->item_name = malloc(ITEM_NAME_MAX);

    return itp;
}

void item_free(item *itp) {
    if (!itp)
        return;

    if (!itp->item_name)
        return;

    free(itp->item_name);
    free(itp);
}

void item_set_name(item *itp, char * name) {
    assert(itp != NULL);
    itp->item_name = name;
}

void item_set_name_deep(item *itp, const char *const name, const size_t len) {
    assert(itp->item_name != NULL);
    assert(name != NULL);   /* Represents an incorrect call */
                            /* see item_set_name */
    assert(len <= ITEM_NAME_MAX);

    strncpy(itp->item_name, name, len);
    /* Insert null byte if not present at correct location */
    if (strlen(name) > len - 1 && name[len - 1] != '\0') {
        /* I believe there is an edge-case if name is 'full' */
        itp->item_name[len - 1] = '\0';
    }
}

void item_print_fancy(item *itp, long long print_flags) {
    int is_tty = isatty(STDOUT_FILENO);

    if (print_flags & ITEM_PRINT_ID) {
        /* ID */
        if (is_tty)
            printf(_ITEM_PRINT_ID_COL "%d\t" _ITEM_PRINT_RESET_COL,
                   itp->item_id);
        else
            printf("%d\t", itp->item_id);
    }
    if (print_flags & ITEM_PRINT_NAME) {
        /* Name */
        if (is_tty)
            printf("%s%s " _ITEM_PRINT_RESET_COL,
                   _ITEM_PRINT_ST_TO_COL(itp->item_st),
                   itp->item_name);
        else 
            printf("%s ", itp->item_name);
    }

    printf("\n");
    fflush(stdout);
}
