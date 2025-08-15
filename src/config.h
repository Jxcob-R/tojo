#ifndef CONFIG_H
#define CONFIG_H

/* Version as a string */
#define CONF_VERSION "0.6"

/* Name definitions */
#define CONF_NAME_LOWER "tojo"
#define CONF_NAME_UPPER "TOJO"
#define CONF_CMD_NAME "tojo"
#define CONF_CMD_NAME_SHORT "tj"

/* Project definitions */
#define CONF_PROJ_DIR ".tojo"

#define CONF_DIR_PERMS 0755

/* GitHub and contributing */
#define CONF_GITHUB "https://github.com/Jxcob-R/tojo"

/* Program return codes */
#define RET_NO_ARGS 1
#define RET_INVALID_OPTS 2
#define RET_INVALID_CMD 3
#define RET_INIT_TJ_EXISTS 4
#define RET_UNABLE_TO_INIT 5
#define RET_NO_PROJ 6

/* Directory search macros */
#define MAX_PATH 4096
#define MAX_PATH_LVLS 128

#define HEX_LEN(type) (sizeof(type) * 2) /* Length of type in hex */

#endif
