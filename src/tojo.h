#ifndef TOJO_H
#define TOJO_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>

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
 * @brief Entry point for command handling
 * @param argc
 * @param argv
 * @return Return code
 */
extern int tj_main(const int argc, char * const argv[]);

#endif
