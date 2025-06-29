#ifndef TRIE_H
#define TRIE_H

#include <assert.h>
#include <stdlib.h>

#include "item.h"

struct prefix_trie_node {
    /* Array of pointers to child nodes in the trie */
    struct prefix_trie_node **children;
    /* Token on node (disreguarded for root) */
    char tok;
    /*
     * Number of initialised children:
     * It is assumed that the children of a prefix_trie_node are initialised in
     * order, from 0, this this effectively provides an insertion position for
     * new children
     */
    unsigned int initialised_children;
};

/**
 * @brief Find the associated array of prefix lengths of each of the strings
 * in strings, such that each prefix is still unique
 * @param strings Array of strings (const char *) of which to find prefixes
 * @param num_strings Number of strings in strings
 * @param len Length of each string
 * @param uniq_chars Number of unique characters possible in a string
 * @param prefix_lengths Array to read into; expected to be the same length as
 * strings.
 * @note All strings are assumed to be unique
 */
extern void shortest_unique_prefix_lengths(const char *const * strings,
                                           const int num_strings,
                                           const int len,
                                           const int uniq_chars,
                                           int * prefix_lengths);

#endif
