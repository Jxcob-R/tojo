#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ds/item.h"

#define LIST_CMD_NAME "list"

extern const char *path; /* Project path used by all functions */

/**
 * @brief Get shortened item codes from the list of items
 * @param items Array of pointers to items which are being listed
 * @return Array of heap-allocated ints associated with the (unique) prefix
 * lengths of each item code
 * @note This is a utility function -- not a command
 * @see shortest_unique_prefix_lengths
 */
extern int * list_item_code_prefixes(item *const *items);

/**
 * @brief Show help for list command
 */
extern void list_help(void);

/**
 * @brief List all tasks in project
 */
extern void list_all_names();

/**
 * @brief Entry point for list command
 * @param argc
 * @param argv Arguments *from* command name (i.e. list <args> ...)
 * @param open_project whether there is an open project
 * @return Exit code
 */
extern int list_cmd(const int argc, char * const argv[],
                    const char *proj_path);

#endif
