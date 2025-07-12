#include "dev-utils/debug-out.h"
#include "graph.h"

/**
 * @brief Free simple edge dependency
 * @param dep Dependency to free
 */
static void free_dependency(struct dependency *dep) {
    (void) dep;
}

/**
 * @brief 
 */
static int add_dependency_edge(struct graph_edge *base_edge,
                               struct graph_edge *add_edge) {
    return 0;
}

/**
 * @brief 
 */
static int add_dependency_item(struct graph_edge *base_edge,
                               const item *const itp) {
    return 0;
}

/**
 * @brief Removes dependency edge in graph from some base edge
 */
static int rm_dependency_edge(struct graph_edge *base_edge,
                              struct graph_edge *added_edge) {
    return 0;
}

/**
 * @brief 
 */
static int rm_dependency_item(struct graph_edge *base_edge, item *itp) {
    return 0;
}

struct dependency * graph_new_dependency(const sitem_id from,
                                         const sitem_id to,
                                         const sitem_id is_ghost) {
    return NULL;
}

int graph_new_dependency_to_arr(struct dependency_set *set,
                            const struct dependency *const new_dependency) {
    return 0;
}


void graph_free_dependency_arr(struct dependency_set *set) {
}

struct graph_edge_list * graph_create_graph(const item *const *items,
                                            const item *base_item,
                                            struct dependency_set *set) {
    return NULL;
}

int graph_item_has_dependency(const struct dependency_set *set,
                              const item *itp) {
    return 0;
}
