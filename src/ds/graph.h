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
    /* The "dependent" item ID */
    sitem_id to;
    /* The "independent" item ID (i.e. the one to *depends on*) */
    sitem_id from;
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
 * @note For implementation, each 'row' of the matrix represents the
 * out-vertices
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
extern struct dependency *graph_new_dependency(const sitem_id from,
                                               const sitem_id to,
                                               const sitem_id is_ghost);

/**
 * @brief Add new dependency struct to the list of current dependencies
 * @return 0 if addition is successful
 * @return -1 in case of some error
 * @note turns the pointer referencing new_dependency to NULL
 */
extern int graph_new_dependency_to_list(struct dependency_list *list,
                                        struct dependency **new_dependency);

/**
 * @brief Test if two dependency structs hold the same data
 * @param a First dependency
 * @param b Second dependency
 * @return 1 if data is equal
 * @return 0 if not
 * @return -1 if the pointers are the same
 */
extern int graph_dependencies_equal(struct dependency *a, struct dependency *b);

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
 * @brief Find the dependency in the dependency list
 * @return Index of position in the list if found
 * @return -1 if not present
 * @see graph_item_has_dependency
 * @note Sets caller's target_dep pointer to NULL *if* the dependecy is found
 */
extern long graph_find_dependency(const struct dependency_list *list,
                                  struct dependency **target_dep);
/**
 * @brief Free array of pointers to dependencies and all associated resources
 * @param list Dependency list to free (pointer set to NULL)
 */
extern void graph_free_dependency_list(struct dependency_list **list);

/**
 * @brief Remove dependencies from a list that also appear in another
 * dependency list.
 * @param list Pointer to pointer of list -- modifies pointer appropriately
 * @param reference_list Constant list of references to dependencies to check
 * duplicates for
 * @note Frees original list
 * @see graph_free_dependency_list
 * @return New allocation for list with appropriate modifications
 * @note (implementation) implemented with a n^2 iteration approach.
 */
extern struct dependency_list *
graph_remove_duplicates(struct dependency_list **list,
                        const struct dependency_list *reference_list);

/**
 * @brief Free a graph of edges between items
 * @param graph List of a base edges in the directed graph to free
 */
extern void graph_free_graph(struct graph_of_items **graph);

/**
 * @brief Create dependency graph from list of items and list of dependencies
 * *upwards* from a given target item
 * @param items Pointer to array of items terminated by a NULL pointer, is set
 * to NULL after call.
 * @param list List of edges found, the final DAG will constitute some *subset*
 * of this list is set to NULL after function call
 * @return Graph as adjacency matrix
 */
extern struct graph_of_items *graph_create_graph(item ***items,
                                                 struct dependency_list **list);

/**
 * @brief Obtain the relevant subgraph of the DAG super_graph that contains the
 * item with some target ID and all parent items and relevant edges.
 * @note The resultant graph only contains a *single* 'sink'/leaf item (that is,
 * the target item)
 * @param super_graph DAG from which to obtain the sub-graph, the pointer to
 * this graph is set to NULL after the call
 * @param target_id ID of target item; i.e. "leaf" of the resultant sub-graph
 * @return New allocated graph of items
 */
extern struct graph_of_items *
graph_get_subgraph_to_item(struct graph_of_items **super_graph,
                           sitem_id target_id);

/**
 * @brief Print each node and edge in the DAG using item format/fancy printing
 * @see item_print_fancy
 * @param dag DAG of items to print
 * @param print_flags Flags to pass to item_print_fancy
 */
extern void graph_print_dag_with_item_fields(const struct graph_of_items *dag,
                                             sitem_id target,
                                             uint64_t print_flags);
#endif
