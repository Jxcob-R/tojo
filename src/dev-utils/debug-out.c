#include "debug-out.h"

/**
 * @brief Print debug standard prefix to stderr
 */
static void print_prefix() {
    fprintf(stderr, ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, DEBUG_PREFIX);
}

/**
 * @brief Print num_chars characters of a message to stderr
 * @param err_msg Message to print
 * @param num_chars Number of characters to print, prints the whole string if
 * set to some negative number.
 */
static void print_msg(const char *err_msg, int num_chars) {
    int msg_len = strlen(err_msg);

    if (strlen(err_msg) >= (size_t) num_chars) {
        fprintf(stderr, "%.*s", num_chars, err_msg);
    } else {
        /* Fill output with */
        int padding = num_chars - msg_len;
        fprintf(stderr, "%s%*s", err_msg, padding, "");
    }
}

/**
 * @brief Print debug standard suffix to stderr
 */
static void print_suffix() {
    fprintf(stderr, ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, DEBUG_SUFFIX);
}

void log_err(const char *err_msg) {
    if (!err_msg) {
        print_prefix();
        print_msg("(No message specified)", DEBUG_LINE_LIMIT);
        print_suffix();
    }

    /* Print log line-by-line */
    const char *line_start = err_msg;
    const char *current = err_msg;
    int line_number = 1;

    while (*current != '\0') {
        if (*current == '\n' || current - line_start == DEBUG_LINE_LIMIT) {
            print_prefix();
            print_msg(line_start, (int) (current - line_start));
            print_suffix();

            line_start = current + 1;
            line_number++;
        }
        current++;
    }

    if (line_start < current) {
        /* Print final debug output */
        print_prefix();
        print_msg(line_start, DEBUG_LINE_LIMIT);
        print_suffix();
    }
    /* Separate logs with a new line */
    fprintf(stderr, "\n");
}

void announce_debugging() {
    log_err("This is the DEBUG build");
}
