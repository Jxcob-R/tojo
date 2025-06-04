#include "config.h"
#include "tojo.h"
#ifdef DEBUG
#include "dev-utils/debug-out.h"
#endif

/**
 * @brief print Return code to stdout
 * @param ret return code
 * @return return code provided
 */
#ifdef DEBUG
static inline int print_return(int ret) {
    fprintf(stdout, "\nRETURN CODE: %d\n", ret);
    return ret;
}
#endif

int main(int argc, char *argv[]) {
#ifdef DEBUG
    announce_debugging();
#endif
    if (argc == 1) {
        printf("Unknown usage of %s\n", argv[0]);
        tj_help();
#ifdef DEBUG
        return print_return(TJ_RET_NO_ARGS);
#else
        return RET_NO_ARGS;
#endif
    }

#ifdef DEBUG
    return print_return(tj_main(argc, argv));
#else
    return tj_main(argc, argv);
#endif
}
