#include "dev-utils/debug-out.h"
#include "trie.h"

/**
 * @brief Make an array of node pointers num long
 * @param num Length of pointer array
 * @return Pointer to head of array
 */
static inline struct prefix_trie_node ** make_child_ptrs(unsigned int num) {
    return (struct prefix_trie_node **) malloc(
        sizeof(struct prefix_trie_node *) * num);
}

/**
 * @brief Initialise a prefix trie node
 * @return Prefix trie node with set of children set to NULL (uninitialised)
 * @return NULL if allocation fails
 * @see malloc
 */
static struct prefix_trie_node * init_node(const int num_children) {
    struct prefix_trie_node *node = (struct prefix_trie_node *)
                                     malloc(sizeof(struct prefix_trie_node));

    if (!node)
        return node;

    node->initialised_children = 0;
    if (num_children == 0) {
        /* No children */
        node->children = NULL;
        node->tok = '\0';
    } else if (num_children > 0) {
        node->children = make_child_ptrs(num_children);
        if (!node->children) { /* Unlikely */
#ifdef DEBUG
            log_err("Trie node could not be initialised with children");
#endif
            return node;
        }
        for (int i = 0; i < num_children; i++) {
            /* Children yet to be initialised */
            node->children[i] = NULL;
        }
        node->tok = '\0';
    }

    return node;
}

/**
 * @brief Free a node and associated resources
 * @param node Node to free
 * @note children are 'let' go, this may result in a memory leak if called
 * incorrectly; use carefully!
 */
static void free_node(struct prefix_trie_node *node) {
    if (!node) return;

    free(node->children);
    free(node);
}

/**
 * @brief Free the trie and associated resources
 * @param root Root of the trie
 */
static void free_trie(struct prefix_trie_node *root) {
    if (!root)
        return;

    for (unsigned int i = 0; i < root->initialised_children; i++) {
        free_trie(root->children[i]);
    }
    
    free_node(root);
}

/**
 * @brief Add a child, involves initialising a new node with 0 children
 * appropriately
 * @param parent Parent to which to add new child
 * @param tok Token (char) of new child
 * @param max Maximum number of children available in the parent
 */
static void add_child(struct prefix_trie_node *parent, const char tok,
                      unsigned int max) {
    /* No children initialised for child by default */
    struct prefix_trie_node *child = init_node(0);

    if (!child) {
#ifdef DEBUG
        log_err("Child of trie could not be added -- malloc failed");
#endif
        return;
    }

    /* Insertion position */
    const unsigned pos = parent->initialised_children;

    if (pos >= max) {
#ifdef DEBUG
        log_err("Position too large for trie node entry");
#endif
        return;
    }

    if (!parent->children) {
        /* Initialise child pointers in parent (if non-existent) */
        parent->children = make_child_ptrs(max);
    }

    child->tok = tok;
    parent->children[pos] = child;
    parent->initialised_children++;
}

/**
 * @brief Find the child of the trie node with the target token
 * @param parent Node to find child of
 * @param tok Target token
 * @return Pointer to child
 * @return NULL if the parent does not have a matching child
 */
static struct prefix_trie_node *get_child(const struct prefix_trie_node *parent,
                                          const char tok) {
    struct prefix_trie_node *curr_child = NULL;
    unsigned int num_children = parent->initialised_children;
    for (unsigned int i = 0; i < num_children; i++) {
        curr_child = parent->children[i];
        if (curr_child->tok == tok)
            return curr_child;
    }
    return NULL;
}

void shortest_unique_prefix_lengths(const char *const *strings,
                                     const int num_strings,
                                     const int len, const int uniq_chars,
                                     int *prefix_lengths) {
    if (!strings) {
        return;
    }

    /* Result array */
    assert(prefix_lengths != NULL);

    /* Initialise trie */
    struct prefix_trie_node *const root = init_node(uniq_chars);
    assert(root != NULL);
    struct prefix_trie_node *curr;

    for (int i = 0; i < num_strings; i++) {
        curr = root;
        int prefix_len = 0;
        for (int j = 0; j < len; j++) {
            assert(strings[i]);
            assert(strings[i][j] != '\0');

            prefix_len++;
            struct prefix_trie_node *new_child = get_child(curr, strings[i][j]);

            if (!new_child) {
                /* Sequence is new -- we can move to the next string */
                add_child(curr, strings[i][j], uniq_chars);
                prefix_lengths[i] = prefix_len;
                break;
            } else {
                /* Continue down trie */
                curr = new_child;
            }
        }
    }

    free_trie(root);
}
