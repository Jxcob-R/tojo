#ifndef WORK_H
#define WORK_H

#include <getopt.h>

#include "ds/item.h"

#define WORK_CMD_NAME "work"

/**
 * @brief Show help for work command
 */
extern void work_help(void);

/**
 * @brief Work on an item; promote to In progress status
 * @param id_str String form of id -- expected to be base 10
 */
extern void work_on_item_id(const char *id_str);

/**
 * @brief Work on an item, specifying its unique code
 * @param code The code of the item to work on -- may be prefix (not full)
 */
extern void work_on_item_code(const char *code);

/**
 * @brief work command
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int work_cmd(const int argc, char *const argv[], const char *proj_path);

#endif
