/**
 * Define helper macros for more convenient testing methods on potentially
 * private source file data
 */
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

/**
 * Functions should use static_fn, such that they can be exposed publicly for
 * unit-testing purposes, otherwise, they are still only static.
 *
 * This idea was inspired by JaredPar's answer at
 * https://stackoverflow.com/questions/593414/how-to-test-a-static-function
 * NOTE: This stipulates that static functions are also conditionally declared
 * in associated .h files
 */
#ifndef TJUNITTEST
#define static_fn static
#else
#define static_fn
#endif

#endif
