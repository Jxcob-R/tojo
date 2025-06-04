#ifndef INIT_H
#define INIT_H

#include <assert.h>
#include <getopt.h>
#include <string.h>

#define INIT_CMD_NAME "init"

/**
 * @brief Show help for init command
 */
extern void init_help(void);

/**
 * @brief Create a standard project directory with required files and data
 * @return 0 on success, -1 on failure with errno set
 * @see dir_init
 */
extern int init_create_project(void);

/**
 * @brief Init command -- intialises a new project
 */
extern int init_cmd(const int argc, char *const argv[], const char *proj_path);

#endif
