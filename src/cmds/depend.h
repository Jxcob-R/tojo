#ifndef DEP_H
#define DEP_H

#include <getopt.h>

#include "ds/graph.h"

#define DEP_CMD_NAME "dep"

/**
 * Dependency string format delimiters
 * These are guaranteed not to be valid hex characters
 * These are also guaranteed to have a length of 1
 */
#define DEP_DELIM ":"
#define DEP_SIBLING_DELIM ","

/**
 * @brief Show help for work command
 */
extern void dep_help(void);

/**
 * @brief Add an item dependency to the project given associated IDs
 * @param dep_str Dependency string with particular formatting expectations:
 * FORM: "a:b[,x]"
 * - This represents adding some item b and any other items "x" as a
 *   dependency/ies to some item a, 
 * - Each item is represented by its ID
 * @note Strings of an invalid form will do nothing
 */
extern void dep_add_ids(const char *dep_str);

/*
 * @brief depend command
 * @param argc
 * @param argv
 * @param proj_path
 * @return return code
 */
extern int dep_cmd(const int argc, char * const argv[],
                    const char *proj_path);

#endif
