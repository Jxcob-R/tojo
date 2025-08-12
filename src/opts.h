#ifndef OPTS_H
#define OPTS_H

#include <assert.h>
#include <getopt.h>
#include <unistd.h>

/**
 * @brief Option-to-function map structure.
 * @note An array of opt_fn is expected to be terminated by a {0, 0, 0} struct
 */
struct opt_fn {
    char short_name; /* Option short name */

    /*
     * Function pointers -- mutually exclusive, i.e., if one is set, the
     * other MUST be NULL
     */
    void (*func_noarg)(void); /* Function pointer for no_argument opts */

    /* Function pointer for options with arguments */
    void (*func_args)(const char *);
};

/**
 */
extern int opts_find_opt_fn_index(const struct opt_fn *opts_fns, const char c);

/**
 */
extern void opts_run_fn(const struct opt_fn *opt, const char *args);

/**
 * @brief Handle all avaiable options in argc and execute associated functions
 */
extern int opts_handle_opts(const int argc, char *const argv[],
                            const char *short_opts,
                            const struct option *long_opts,
                            const struct opt_fn *opt_fns);

#endif
