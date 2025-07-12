#ifndef GRAPH_H
#define GRAPH_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "item.h"

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
 * @brief A set of dependency 'edges' (NOT graph edges), contains allocated
 * resources for these basic edges
 */
struct dependency_set {
    struct dependency **dependencies;
    int count;
    unsigned int capacity; /* In *elements* (NOT bytes) */
};

struct graph_edge_list {
    /* Pointers to the next edges in the graph (from head_item) */
    struct graph_edge **edges;
    /* Number of following edges */
    uint16_t num_edges;
    uint16_t capacity; /* In *elements* (NOT bytes) */
};

/**
 * @brief Directed graph edge representing dependency between items allocated
 * in memory.
 * @note The convention is to use the 'base' of the dependency DAG to represent
 * the graph 'upwards'
 */
struct graph_edge {
    /* Item at the head of the directed dependency edge */
    item *head_item;
    struct graph_edge_list next_edges;
    /*
     * If head_item is dependent, then this union will represent the previous
     * dependency edges as a heap-allocated array, otherwise, it will represent
     * an 'independent' source in the DAG
     */
    union {
        struct graph_edge_list prev_edges;
        item *source;
    };
    uint8_t has_source;
    /* See struct dependency */
    int is_ghost;
};

/**
 * @brief Create new simple edge dependency given 2 item IDs and ghost status
 */
extern struct dependency * graph_new_dependency(const sitem_id from,
                                                const sitem_id to,
                                                const sitem_id is_ghost);

/**
 * @brief Add new dependency struct to the set of current dependencies
 * @return 0 if addition is successful
 * @return -1 in case of some error
 */
extern int
graph_new_dependency_to_arr(struct dependency_set *set,
                            const struct dependency *const new_dependency);

/**
 * @brief Free array of pointers to dependencies and all associated resources
 */
extern void graph_free_dependency_arr(struct dependency_set *set);

/**
* @brief Create dependency graph from list of items and set of dependencies
* *upwards* from a given target item
* @param items List of items terminated by a NULL pointer
* @param base_item Base item (must be in items) from which to construct DAG
* @param set Set of edges found, the final DAG will constitue some *subset* of
* this set of dependencies
* @note Graph edges point to items *through* their position in items. 
* @return Graph edges directed to the base item as an edge list
*/
extern struct graph_edge_list * graph_create_graph(const item *const *items,
                                                   const item *base_item,
                                                   struct dependency_set *set);

/**
* @brief Check if an item has any dependencies listed in the dependency set
* @param set Set of dependencies to search
* @param itp Item to find dependency for
* @return 1 if item has dependency
* @return 0 if item does not have dependency
* @note That 0 can be returned either if the item is a source in the DAG
* constructed by the dependency set, or is not present at all.
*/
extern int graph_item_has_dependency(const struct dependency_set *set,
                                     const item *itp);
#endif
