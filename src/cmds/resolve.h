#ifndef RES_H
#define RES_H

#include <getopt.h>

#include "ds/item.h"

#define RES_CMD_NAME "res"

/**
 * @brief Show help for resolve command
 */
extern void res_help(void);

/**
 * @brief Mark item as done
 * @param id String form of id -- expected to be base 10
 */
extern void res_item_id(const char *id_str);

/**
 * @brief Mark item with specified code as done
 * @parma code Code of item to complete
 */
extern void res_item_code(const char *code);

/**
 * @brief Resolve command; mark item as done
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int res_cmd(const int argc, char *const argv[], const char *proj_path);

#endif
