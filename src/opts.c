#include "opts.h"

static int terminating_opt_fn(const struct opt_fn *const fn) {
    return 
        fn->short_name == 0
    && fn->func_noarg == NULL
    && fn->func_args == NULL;
}

int opts_find_opt_fn_index(const struct opt_fn *opt_fns, const char opt) {
    int index = -1;

    for (int i = 0; !terminating_opt_fn(&opt_fns[i]); i++) {
        if (opt == opt_fns[i].short_name) {
            index = i;
            break;
        }
    }

    return index;
}

void opts_run_fn(const struct opt_fn *opt, const char *args) {

    /* Run appropriate function */
    if (opt->func_args) {
        assert(opt->func_noarg == NULL);

        opt->func_args(args);
    } else if (opt->func_noarg) {
        assert(opt->func_args == NULL);

        opt->func_noarg();
    }
}

int opts_handle_opts(const int argc, char * const argv[],
                     const char *short_opts,
                     const struct option *long_opts,
                     const struct opt_fn *opt_fns) {
    int opts_handled = 0;

    int c = 0;
    int option_index = 0;

    int options_exhausted = 0;

    while ((c = getopt_long(argc, argv, short_opts, long_opts,
                            &option_index)) != -1 && !options_exhausted) {

        /* Call correct option function with arguments */
        int opt_fn_index = opts_find_opt_fn_index(opt_fns, c);
        if (opt_fn_index >= 0)
            opts_run_fn(&opt_fns[opt_fn_index], optarg);
        
        if (opts_handled != -1)
            opts_handled++;
    }

    return opts_handled;
}
