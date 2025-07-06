#include "item.h"
#include "dev-utils/debug-out.h"

/**
 * @brief Characters from which an item's code can be generated
 * @note This is guaranteed to have a length ITEM_CODE_CHARS
 * @note This is a null-terminated string
 */
const char item_code_chars[ITEM_CODE_CHARS + 1] = "abcdefghijklmnopqrstuvwxyz";

item * item_init() {
    item *itp = (item *) malloc(sizeof(item));

    itp->item_name = malloc(ITEM_NAME_MAX);

    return itp;
}

item **item_array_init(int num_items) {
    item **items = (item **) malloc(sizeof(item *) * (num_items + 1));
    if (items) {
        for (int i = 0; i < num_items; i++) {
            items[i] = item_init();
        }
    }
    items[num_items] = NULL;
    return items;
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

/**
 * @brief Trim leading and trailing whitespace from name
 * @param name String to trim
 * @return Start of string -- new
 */
static char * trim_name_whitespace(char *name) {
    /* Trim leading whitespace */
    while (isspace(*name))
        name++;
    if (*name == '\0') {
#ifdef DEBUG
        log_err("Name is just whitespace");
#endif
        return name;
    }

    /* Trim trailing whitespace */
    char* end = name + strlen(name) - 1;
    while (end >= name && isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';

    return name;
}

void item_set_name_deep(item *itp, const char *name, const size_t len) {
    assert(itp->item_name != NULL);
    assert(name != NULL);   /* Represents an incorrect call */
                            /* see item_set_name */
    assert(len <= ITEM_NAME_MAX);

    char *new_name = malloc((strlen(name) + 1) * sizeof(char));
    if (!new_name) return;
    strcpy(new_name, name);
    char *new_start = trim_name_whitespace(new_name);

    if (new_start - new_name > 0) {
        char *tmp = malloc((strlen(new_start) + 1) * sizeof(char));
        if (!tmp) {
            free(new_name);
            return;
        }
        strcpy(tmp, new_start);
        free(new_name);
        new_name = tmp;
        new_start = NULL;
    }

    strncpy(itp->item_name, new_name, len);
    /* Insert null byte if not present at correct location */
    if (strlen(new_name) > len - 1 && new_name[len - 1] != '\0') {
        /* I believe there is an edge-case if the name is 'full' */
        itp->item_name[len - 1] = '\0';
    }
    free(new_name);
}

void item_set_code(item *itp) {
    /* Use "unitialised" code for the item */
    if (itp->item_id == -1) {
#ifdef DEBUG
        log_err("Code attempting to be set on an item with ID of -1");
#endif
        return;
    }

    assert(itp->item_id >= 0);

    /*
     * NOTE:
     * This generator must be linearly independent modulo ITEM_CODE_CHARS and
     * greater than ITEM_CODE_CHARS ^ ITEM_CODE_LEN.
     * Large primes are thus the best candidate for this task.
     */
    const unsigned int generator = 1225022963;
    uint64_t code_index = itp->item_id * generator;

    for (int i = 0; i < ITEM_CODE_LEN; i++) {
        itp->item_code[i] = item_code_chars[code_index % ITEM_CODE_CHARS];
        code_index /= ITEM_CODE_CHARS;
    }
}

int item_is_valid_code(const char *code) {
    if (!code) {
#ifdef DEBUG
        log_err("Code provided was NULL");
#endif
        return -1;
    }

    for (int i = 0; i < ITEM_CODE_LEN && code[i] != '\0'; i++) {
        if (!strchr(item_code_chars, code[i])) {
            return 0;
        }
    }

    return 1;
}

void item_print_fancy(const item *itp, long long print_flags, void *arg) {
    int is_tty = isatty(STDOUT_FILENO);

    if (print_flags & ITEM_PRINT_ID) {
        /* ID */
        if (is_tty)
            printf(_ITEM_PRINT_ID_COL "%d\t" _ITEM_PRINT_RESET_COL,
                   itp->item_id);
        else
            printf("%d\t", itp->item_id);
    }
    if (print_flags & ITEM_PRINT_CODE) {
        assert(arg); /* Avoid segfault */
        /* Use arg */
        int highlight_chars = *(int *)arg;
        if (highlight_chars > ITEM_CODE_LEN)
            highlight_chars = ITEM_CODE_LEN;
        else if (highlight_chars < 0)
            highlight_chars = 0;

        if (is_tty)
            printf(
                "%s%.*s" _ITEM_PRINT_RESET_COL
                _ITEM_PRINT_CODE_INACTIVE_COL "%.*s " _ITEM_PRINT_RESET_COL,
                _ITEM_PRINT_ST_TO_COL(itp->item_st),
                highlight_chars, itp->item_code,
                ITEM_CODE_LEN - highlight_chars,
                itp->item_code + highlight_chars
            );
        else 
            printf("%s ", itp->item_name);
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
