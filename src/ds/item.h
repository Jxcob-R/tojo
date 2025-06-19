#ifndef ITEM_H
#define ITEM_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ITEM_NAME_MAX 256 /* Maximum item name length */

/**
 * Signed item ID type
 */
typedef int32_t sitem_id;

typedef struct item item;

/**
 * @brief Struct of a single todo item
 */
struct item {
    sitem_id item_id;
    char *item_name;
    enum status {
        TODO,
        IN_PROG,
        DONE,
        ITEM_STATUS_COUNT, /* Can later be constant */
    } item_st;
    /*
     * Future additions
     * File or directory target name in project -- relative to project root
    char item_target[MAX_PATH];
     * Date and time item was last changed
    time_t item_time;
     * Priority of an item
     enum priority {
        LOW,
        MED,
        HIGH
     } item_priority;
     */
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
 * @note Will insert null byte if the string name of size len characters is
 * not already null terminated (that is if name[len - 1] != '\0')
 */
extern void item_set_name_deep(item *itp, const char *const name,
                               const size_t len);

/* Print styling using ANSI colours */
#define _ITEM_PRINT_ID_COL "\x1b[1m"

/* Status colours from enum status */
#define _ITEM_PRINT_ST_TO_COL(st) \
    ((const char*[]){"\x1b[33m", "\x1b[32m", "\x1b[34m"})[st]

/* Reset colour */
#define _ITEM_PRINT_RESET_COL "\x1b[0m"

/* Print flags */
#define ITEM_PRINT_ID (1 << 0)
#define ITEM_PRINT_NAME (1 << 1)
#define ITEM_PRINT_STATUS (1 << 2) /* Currently unused */

/**
 * @brief Print the content of the item pointed to by itp given by print_flags
 * @param itp Pointer to item
 * @param print_flags Flags specifying print style
 */
extern void item_print_fancy(item *itp, long long print_flags);

#endif
