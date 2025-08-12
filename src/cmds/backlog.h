#ifndef BACK_H
#define BACK_H

#include <getopt.h>

#include "ds/item.h"

#define BACK_CMD_NAME "back"

/**
 * @brief Show help for back command
 */
extern void back_help(void);

/**
 * @brief Backlog an item given its id
 * @param id_str String form of id
 */
extern void back_item_id(const char *id_str);

/**
 * @brief Backlog an item given its item code
 * @param code Code, or code prefix of item to restage
 */
extern void back_item_code(const char *code);

/**
 * @brief back command
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int back_cmd(const int argc, char *const argv[], const char *proj_path);

#endif
