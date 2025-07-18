#ifndef ITEM_H
#define ITEM_H

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ITEM_NAME_MAX 256 /* Maximum item name length */

#define ITEM_CODE_LEN 7 /* Length of an item code */
#define ITEM_CODE_CHARS 26 /* Number of usable item code characters */

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
    char item_code[ITEM_CODE_LEN];
    char *item_name;
    enum status {
        BACKLOG,
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
 * @brief Allocate heap memory for a number of items and place pointers in array
 * @param num_items Number of items to allocate heap space for
 * @return Array of num_items pointers to items terminated by a NULL pointer
 * @warning Pointer returned may be NULL in case of failed malloc call
 */
extern item ** item_array_init(int num_items);

/**
 * @brief Allocate heap memory for an array of item pointers, and set all
 * positions as NULL
 * @param num_items Number of items to allocate heap space for
 * @return Array of num_items pointers, all pointing to NULL
 * @warning Pointer returned may be NULL in case of failed malloc call
 */
extern item ** item_array_init_empty(int num_items);

/**
 * @brief Reallocates the array of item pointers with a size of nmemb pointers
 * *plus* a terminating NULL pointer.
 * @param items Array of item pointers to resize
 * @param num_items The new number of items to hold in the items array
 * @return Pointer to new start of memory buffer
 * @return NULL if realloc call fails
 */
extern item ** item_array_resize(item **items, int num_items);

/**
 * @brief Find item with given ID in items pointer array
 * @param items Array of item pointers
 * @param id Target ID to find
 * @return Index of item with given ID in items
 */
extern size_t item_array_find(const item *const *items, sitem_id id);

/**
 * @brief Count the number of item pointers in an array
 * @param items Array of item pointers
 * @return Size of items
 * @return 0 if items is NULL
 */
extern size_t item_count_items(item *const *items);

/**
 * @brief Move a given number of pointers of items from a source to a
 * destination or until source is exhausted
 * @param items_dest Destination of items
 * @param items_src Source of items to copy, should be terminated by a NULL
 * pointer
 * @param n At most number of items to copy
 * @note Copying and Adding occurs from the start of both pointers
 * @warning May overflow
 */
extern void item_array_add(item **items_dest, item *const *items_src,
                              const size_t n);

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
extern void item_set_name_deep(item *itp, const char *name,
                               const size_t len);

/**
 * @brief Sets a unique ITEM_CODE_LEN-lengthed code for an item. This is unique
 * on the basis of IDs (see sitem_id)
 * @param itp Pointer to item of which to set the code
 */
extern void item_set_code(item *itp);

/**
 * @brief Check that the code provided is a valid item code, i.e. contains only
 * valid characters and is *less than or equal to* a full code length.
 * @param code Code-candidate, is null-terminated
 * @return 1 if code is valid
 * @return 0 if invalid
 * @return -1 on error
 */
extern int item_is_valid_code(const char *code);

/* Print styling using ANSI colours */
#define _ITEM_PRINT_ID_COL "\x1b[1m"

/* Status colours from enum status */
#define _ITEM_PRINT_ST_TO_COL(st) \
    ((const char*[]){"\x1b[41m", "\x1b[33m", "\x1b[32m", "\x1b[34m"})[st]

#define _ITEM_PRINT_CODE_INACTIVE_COL "\x1b[90m" 

/* Reset colour */
#define _ITEM_PRINT_RESET_COL "\x1b[0m"

/* Print flags */
#define ITEM_PRINT_ID (1 << 0)
#define ITEM_PRINT_NAME (1 << 1)
#define ITEM_PRINT_CODE \
    (1 << 2) /* Expects an int as arg, where 0 < arg < ITEM_CODE_LEN */

/**
 * @brief Print the content of the item pointed to by itp given by print_flags
 * @param itp Pointer to item
 * @param print_flags Flags specifying print style
 * @param arg Pointer to some argument corresponding with a print mode. Note
 * incompatibilities and expected types
 */
extern void item_print_fancy(const item *itp, long long print_flags, void *arg);

#endif
