#ifndef ADD_H
#define ADD_H

#include <getopt.h>

#include "ds/item.h"

#define ADD_CMD_NAME "add"

extern item it;

/**
 * @brief Show help for add command
 */
extern void add_help(void);

/**
 * @brief add name to item
 */
extern void add_item_name(const char *name);

/**
 * @brief Add command
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int add_cmd(const int argc, char * const argv[], const char *proj_path);

#endif
