#include "dev-utils/debug-out.h"
#include "ds/item.h"
#include "item.h"
#include "graph.h"

/**
 * @brief Free simple edge dependency
 * @param dep Dependency to free
 */
static void free_dependency(struct dependency *dep) {
    free(dep);
}

/**
 * @brief Return new dependency on the heap; helper for
 * graph_init_dependency_list
 * @param from 
 * @param to 
 * @param is_ghost 
 * @return Pointer to newly allocated dependency
 * @return NULL if malloc fails
 */
static struct dependency * new_dependency(const sitem_id from,
                                          const sitem_id to,
                                          const int is_ghost) {
    struct dependency *d = malloc(sizeof(struct dependency));
    d->from = from;
    d->to = to;
    d->is_ghost = is_ghost;
    return d;
}

/**
 * @brief Double the size of the list
 * @param list Dependency list to grow in memory
 */
static void grow_list(struct dependency_list *list) {
    void *ptr = NULL;
    if (list->capacity == 0)
        list->capacity = GRAPH_INIT_CAPACITY;
    while (!ptr)
        ptr = realloc(list, sizeof(struct dependency_list) *
                      list->capacity * 2);
    list->dependencies[list->capacity] = NULL;
    list->capacity *= 2;
    list->dependencies = ptr;
}

/**
 * @brief Check if the given dependency list is at capacity
 * @param list to check
 * @return 1 if capacity is reached
 * @return 0 if not
 */
static int list_at_capacity(const struct dependency_list *const list) {
    return list->capacity == list->count;
}

/**
 * @brief Create adjacency matrix from given list and provide it to the given
 * graph @param graph Graph to populate
 * @param list List of dependencies from which to generate the adjacency matrix
 */
static void create_matrix(struct graph_of_items *graph,
                          const struct dependency_list *list) {
    assert(graph->item_list);
    assert(graph->count > 0);

    size_t matrix_dim = graph->count;

    /* Allocated matrix */
    uint8_t **adj_matrix = malloc(sizeof(uint8_t *) * matrix_dim);
    if (!adj_matrix) {
        graph->adj_matrix = NULL;
        return;
    }

    for (size_t i = 0; i < matrix_dim; i++) {
        adj_matrix[i] = malloc((sizeof(uint8_t) * matrix_dim));
        if (!adj_matrix) {
            return;
        }
    }

    /* Assign edges */
    sitem_id from_id, to_id;
    size_t index_of_from, index_of_to;
    for (unsigned int i = 0; i < list->count; i++) {
        from_id = list->dependencies[i]->from;
        to_id = list->dependencies[i]->to;
        index_of_from = item_array_find((const item *const *)graph->item_list,
                                        from_id);
        index_of_to = item_array_find((const item *const *)graph->item_list,
                                      to_id);
        adj_matrix[index_of_from][index_of_to]++;
    }

    graph->adj_matrix = adj_matrix;
}

/**
 * @brief Check if two given dependency structs have equal content
 */
static int dependencies_are_equal(const struct dependency *first,
                                  const struct dependency *second) {
    return first->from == second->from &&
           first->to == second->to &&
           first->is_ghost == second->is_ghost;
}

//////// TODO: Refactor ////////

// /**
//  * @brief 
//  */
// static int add_dependency_edge(struct graph_edge *base_edge,
//                                struct graph_edge *add_edge) {
//     return 0;
// }
//
// /**
//  * @brief 
//  */
// static int add_dependency_item(struct graph_edge *base_edge,
//                                const item *const itp) {
//     return 0;
// }
//
// /**
//  * @brief Removes dependency edge in graph from some base edge
//  */
// static int rm_dependency_edge(struct graph_edge *base_edge,
//                               struct graph_edge *added_edge) {
//     return 0;
// }
//
// /**
//  * @brief 
//  */
// static int rm_dependency_item(struct graph_edge *base_edge, item *itp) {
//     return 0;
// }
//
struct dependency_list *
graph_init_dependency_list(unsigned int initial_capacity) {
    struct dependency **dep_list = NULL;
    /* Set list itself */
    unsigned int capacity
        = initial_capacity != 0 ? initial_capacity : GRAPH_INIT_CAPACITY;
    dep_list = malloc(capacity * sizeof(struct dependency *));
    dep_list[0] = NULL;
#ifdef DEBUG
    if (!dep_list)
        log_err("graph_init_dependency_list: Malloc failed");
#endif
    /* List data struct */
    struct dependency_list *list = malloc(sizeof(struct dependency_list));
    if (list) {
        list->capacity = capacity;
        list->count = 0;
        list->dependencies = dep_list;
    }
    return list;
}

struct dependency * graph_new_dependency(const sitem_id from,
                                         const sitem_id to,
                                         const sitem_id is_ghost) {
    return new_dependency(from, to, is_ghost);
}

int graph_new_dependency_to_list(struct dependency_list *list,
                                struct dependency **new_dependency) {
    if (list_at_capacity(list)) {
        grow_list(list);
    }
    list->dependencies[list->count] = *new_dependency;
    list->count++;
    *new_dependency = NULL;
    return 0;
}

int graph_dependencies_equal(struct dependency *a, struct dependency *b) {
    assert(a && b);

    if (a == b)
        return -1;

    /* This is sufficient */
    return (a->from == b->from && a->to == b->to);
}

void graph_free_dependency_list(struct dependency_list **list) {
    for (unsigned int i = 0; i < (*list)->count; i++)
        free_dependency((*list)->dependencies[i]);
    free((*list)->dependencies);
    free(*list);
    *list = NULL;
}
struct dependency_list *
graph_remove_duplicates(struct dependency_list **list,
                        const struct dependency_list *reference_list) {
    assert(list && *list);
    if (!reference_list || reference_list->count == 0)
        return *list;

    struct dependency_list *new_list =
        graph_init_dependency_list((*list)->count);
    
    int is_duplicated = 0;
    for (unsigned int i = 0; i < (*list)->count; i++) {
        for (unsigned int j = 0; j < reference_list->count; j++) {
            if (graph_dependencies_equal((*list)->dependencies[i],
                                          reference_list->dependencies[j])) {
                is_duplicated = 1;
                break;
            }
        }
        if (!is_duplicated)
            graph_new_dependency_to_list(new_list, &(*list)->dependencies[i]);
        is_duplicated = 0;
    }

    graph_free_dependency_list(list);
    return new_list;
}

struct graph_of_items *
graph_create_graph(item ***items, struct dependency_list **list) {
    size_t item_count = item_count_items(*items);
    if (item_count == 0)
        return NULL;
    struct graph_of_items *graph = malloc(sizeof(struct graph_of_items));
    graph->count = item_count;
    graph->item_list = *items;

    *items = NULL;
    create_matrix(graph, *list);
    return graph;
}

int graph_item_has_dependency(const struct dependency_list *list,
                              const item *itp) {
    for (unsigned int i = 0; i < list->count; i++) {
        if (list->dependencies[i]->to == itp->item_id)
            return 1;
    }
    return 0;
}

long graph_find_dependency(const struct dependency_list *list,
                           struct dependency **target_dep) {
    for (unsigned int i = 0; i < list->count; i++) {
        if (dependencies_are_equal(*target_dep, list->dependencies[i])) {
            *target_dep = NULL;
            return i;
        }
    }
    return -1;
}

void graph_free_graph(struct graph_of_items **graph) {
    for (size_t i = 0; i < (*graph)->count; i++) {
        free((*graph)->adj_matrix[i]);
    }
    free((*graph)->adj_matrix);
    free(*graph);
    *graph = NULL;
}
