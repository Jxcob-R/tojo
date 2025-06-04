#ifndef ITEM_H
#define ITEM_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct item item;

/**
 * @brief Struct of a single todo item
 */
struct item {
    char *item_name;
};

/**
 * @brief Allocate heap memory for an new item
 * @note Item name is also heap allocated
 * @warning Pointer returned may be NULL in case of failed malloc call
 */
extern item * item_init(void);

/**
 * @brief Free an item and all associated resources from memory
 * @note item free *may be* NULL, however, there will be no way of knowing if
 * this was the case when the function was called.
 */
extern void item_free(item *itp);

/**
 * @brief Set the name of an item using a reference to the string provided.
 * @see item_set_name_deep for a heap deep-copy version
 */
extern void item_set_name(item *itp, char *name);

/**
 * @brief Set the name of the item by copying data to heap-memory
 * @see item_set_name for a simple use of the name reference
 */
extern void item_set_name_deep(item *itp, const char *const name);

/* Print styling using ANSI colours */
#define ITEM_PRINT_NAME_COL           "\x1b[32m"
#define ITEM_PRINT_RESET_COL          "\x1b[0m"

/* Print flags */
#define ITEM_PRINT_NAME (1 << 0)

/**
 * @brief Print the content of the item pointed to by itp given by print_flags
 * @param itp Pointer to item
 * @param print_flags Flags specifying print style
 */
extern void item_print_fancy(item *itp, long print_flags);

#endif
