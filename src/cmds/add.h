#ifndef ADD_H
#define ADD_H

#include <ctype.h>
#include <getopt.h>

#include "ds/item.h"

#define ADD_CMD_NAME "add"

/**
 * @brief Show help for add command
 */
extern void add_help(void);

/**
 * @brief Restage an existing item as a TODO status from some other status
 * @param id_str String form of id
 */
extern void add_restage_item_id(const char *id_str);

/**
 * @brief Restage an existing item as a TODO status using its code
 * @param code Code of item to restage
 */
extern void add_restage_item_code(const char *code);

/**
 * @brief Add name to item
 * @param name Name of new item to add
 */
extern void add_item_name(const char *name);

/**
 * @brief Add command
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int add_cmd(const int argc, char *const argv[], const char *proj_path);

#endif
