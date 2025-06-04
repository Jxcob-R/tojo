#ifndef TOJO_H
#define TOJO_H

#define _GNU_SOURCE 1

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>

/* Name definitions */
#define TJ_NAME_LOWER "tojo"
#define TJ_NAME_UPPER "TOJO"
#define TJ_CMD_NAME "tojo"
#define TJ_CMD_NAME_SHORTENED "tj"

#define TJ_DIR_NAME ".tojo"

/* Directory search macros */
#define _MAX_PATH 4096
#define _MAX_PATH_LVLS 128

/* Version as a string */
#define TJ_VERSION "0.0"

/* GitHub and contributing */
#define TJ_GITHUB "https://github.com/..."

/* Program return codes */
#define TJ_RET_NO_ARGS 1
#define TJ_RET_INVALID_OPTS 2
#define TJ_RET_INVALID_CMD 3

/**
 * @brief Command struct representing a single valid command that traces to a
 * module
 */
struct cmd {
    char *cmd_name;
    int (*cmd_fn) (const int, char *const [], const char *);
};

/**
 * @brief Show help page
 */
extern void tj_help(void);

/**
 * @brief Print version number
 */
extern void tj_print_vers(void);

/**
 * @brief Handle options for base command
 * @param argc
 * @param argv
 * @return Number of options handled, negative value in the case of an invalid
 * set of options
 */
extern int tj_handle_opts(const int argc, char * const argv[]);

/**
 * @brief Check if directory is a current project
 * @param dir Write relative project  directory to dir if it exists, leave
 * empty otherwise.
 * otherwise.
 * @return 0 on success, some error value otherwise
 */
extern int tj_get_proj_dir(char *dir);

/**
 * @brief Entry point for command handling
 * @param argc
 * @param argv
 * @return Return code
 */
extern int tj_main(const int argc, char * const argv[]);

#endif
