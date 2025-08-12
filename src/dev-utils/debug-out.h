#ifndef DEBUG_OUT_C
#define DEBUG_OUT_C

#include <stdio.h>
#include <string.h>

/* Printing macros and helpers */
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define DEBUG_PREFIX "=== DEBUG OUTPUT: " /* Printed before debug messages */
#define DEBUG_SUFFIX " ==="               /* Printed after messages */
#define DEBUG_LINE_LIMIT 50               /* Character limit for messages */

/**
 * @brief Log error messages to stderr
 */
extern void log_err(const char *err_msg);

/**
 * @brief Announce debugging build
 */
extern void announce_debugging();

#endif
