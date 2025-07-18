#ifndef GRAPH_H
#define GRAPH_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "item.h"

#define GRAPH_INIT_CAPACITY 16

/**
 * @brief An item dependency represented by a pair of IDs, this can exist
 * separately from actual item structs.
 * @note Dependencies effectively represent directed edges
 */
struct dependency {
    /* The "independent" item ID (i.e. the one that to *depends on*) */
    sitem_id from;
    /* The "dependent" item ID */
    sitem_id to;
    /*
     * Ghosts are created when a dependency is created for an item which is
     * already complete
     */
    int is_ghost;
};

/**
 * @brief A list of dependency 'edges' (NOT graph edges), contains allocated
 * resources for these basic edges, this may be referred to as a 'list' for the
 * sake of brevity.
 */
struct dependency_list {
    struct dependency **dependencies;
    unsigned int count;
    unsigned int capacity; /* In *elements* (NOT bytes) */
};

/**
 * @brief Represent directed graph as an adjancency matrix
 */
struct graph_of_items {
    size_t count;
    item **item_list;
    uint8_t **adj_matrix;
};

/**
 * @brief Initialise empty dependency list
 * @param initial_capacity Initial capacity of the dependency list, if this is 0
 * then GRAPH_INIT_CAPACITY is used for the initial capacity
 * @return Heap-allocated space for a dependency list
 * @note No dependency structs are initialised
 * @note List may be NULL if malloc fails
 */
extern struct dependency_list *
graph_init_dependency_list(unsigned int initial_capacity);

/**
 * @brief Create new simple edge dependency given 2 item IDs and ghost status
 */
extern struct dependency * graph_new_dependency(const sitem_id from,
                                                const sitem_id to,
                                                const sitem_id is_ghost);

/**
 * @brief Add new dependency struct to the list of current dependencies
 * @return 0 if addition is successful
 * @return -1 in case of some error
 */
extern int
graph_new_dependency_to_list(struct dependency_list *list,
                             struct dependency *const new_dependency);

/**
* @brief Check if an item has any dependencies listed in the dependency list
* @param list List of dependencies to search
* @param itp Item to find dependency for
* @return 1 if item has dependency
* @return 0 if item does not have dependency
* @note That 0 can be returned either if the item is a source in the DAG
* constructed by the dependency list, or is not present at all.
*/
extern int graph_item_has_dependency(const struct dependency_list *list,
                                     const item *itp);

/**
 * @brief Free array of pointers to dependencies and all associated resources
 */
extern void graph_free_dependency_list(struct dependency_list **list);

/**
 * @brief Free a graph of edges between items
 * @param graph List of a base edges in the directed graph to free
 */
extern void graph_free_graph(struct graph_of_items **graph);

/**
* @brief Create dependency graph from list of items and list of dependencies
* *upwards* from a given target item
* @param items Pointer to array of items terminated by a NULL pointer, is set to
* NULL after call.
* @param base_item Base item (must be in items) from which to construct DAG
* @param list List of edges found, the final DAG will constitue some *subset* of
* this list is set to NULL after function call
* @return Graph as adjacency matrix
*/
extern struct graph_of_items *
graph_create_graph(item ***items, struct dependency_list **list);
#endif
