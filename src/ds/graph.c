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
 * @brief Check if an edge exists in the adjacency matrix
 * @param adj_mat Adjacency matrix
 * @param from Index of item which the edge goes from
 * @param to Index of item which the edge goes to
 * @return 1 if true
 * @return 0 if not edge exists
 */
static int has_edge(const uint8_t *const *adj_mat, size_t from, size_t to) {
    return adj_mat[from][to] != 0;
}

/**
 * @brief Initialise an n x n matrix
 */
static uint8_t **init_matrix(size_t n) {

    /* Allocated matrix */
    uint8_t **adj_matrix = malloc(sizeof(uint8_t *) * n);
    if (!adj_matrix) {
        return NULL;
    }

    for (size_t i = 0; i < n; i++) {
        adj_matrix[i] = malloc((sizeof(uint8_t) * n));
        if (!adj_matrix[i]) {
            return NULL;
        }
        memset(adj_matrix[i], 0, n);
    }

    return adj_matrix;
}

/**
 * @brief Free adjacency matrix
 * @param mat Matrix on heap to free, set to 
 */
static void free_matrix(uint8_t ***mat, size_t n) {
    for (size_t i = 0; i < n; i++) {
        free((*mat)[i]);
    }
    free(*mat);
    *mat = NULL;
}

/**
 * @brief Initialise a new graph of items with NULL/zeroed fields
 */
struct graph_of_items * init_graph() {
    struct graph_of_items *graph = malloc(sizeof(struct graph_of_items));
    graph->count = 0;
    graph->adj_matrix = NULL;
    graph->item_list = NULL;
    return graph;
}

/**
 * @brief Create adjacency matrix from given list and provide it to the given
 * graph
 * @param graph Graph to populate
 * @param list List of dependencies from which to generate the adjacency matrix
 */
static void create_matrix(struct graph_of_items *graph,
                          const struct dependency_list *list) {
    assert(graph->item_list);
    assert(graph->count > 0);

    size_t matrix_dim = graph->count;
    uint8_t **adj_matrix = init_matrix(matrix_dim);
    if (!adj_matrix)
        return;

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
        /* Mark edge*/
        adj_matrix[index_of_from][index_of_to] = 1;
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
    if (!*new_dependency) {
        return -1;
    }
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
    free_matrix(&(*graph)->adj_matrix, (*graph)->count);
    item_array_free(&(*graph)->item_list, (*graph)->count);
    free(*graph);
    *graph = NULL;
}

struct graph_of_items *
graph_create_graph(item ***items, struct dependency_list **list) {
    size_t item_count = item_count_items(*items);
    if (item_count == 0)
        return NULL;
    struct graph_of_items *graph = init_graph();
    graph->count = item_count;
    graph->item_list = *items;

    *items = NULL;
    create_matrix(graph, *list);
    graph_free_dependency_list(list);
    return graph;
}

/**
 * @brief Reverse DFS (that is, 'against' directed edges) in a DAG, which has
 * already been deconstructed, note the implementation of get_ancestor_dag.
 * @important Edges that are kept in adj_matrix are marked with UINT8_MAX
 * @important Items indices that are kept are marked as non-zero
 * @return The number of nodes found.
 */
static size_t reverse_dag_dfs(uint8_t *const *adj_matrix, uint8_t *visited,
                              size_t start, size_t n) {
    assert(start <= n);

    size_t in_tree_size = 1; /* We count the starting node, so always >= 1 */
    for (size_t i = 0; i < n; i++) {
        if (has_edge((uint8_t const *const *)adj_matrix, start, i) &&
            !visited[i])
        {
            visited[i] = 1;
            adj_matrix[start][i] = UINT8_MAX;
            in_tree_size += reverse_dag_dfs(adj_matrix, visited, i, n);
        }
    }

    return in_tree_size;
}

/**
 * @brief Get matrix of ancestor items in DAG from position i, this is assumed
 * to not contain cycles, but may not be connected.
 * @note This is an exclusive helper for graph_get_subgraph_to_item.
 * @param orig_dag Original DAG with some n items
 * free
 * @param i Target index
 * @return New heap allocated matrix of some m x m size
 */
static struct graph_of_items *
get_ancestor_dag(struct graph_of_items *orig_dag, size_t i) {
    /* Implements a BFS on a directed adjacency matrix */
    const size_t n = orig_dag->count;
    item **items = orig_dag->item_list;
    uint8_t **adj_matrix = orig_dag->adj_matrix;
    orig_dag = NULL;

    uint8_t *items_to_keep = malloc(n);
    if (!items_to_keep) return NULL;
    memset(items_to_keep, 0, n);
    items_to_keep[i] = 1;

    size_t new_size = reverse_dag_dfs(adj_matrix, items_to_keep, i, n);
    item** new_items = item_array_init_empty(new_size);
    uint8_t **new_adj_mat = init_matrix(new_size);

    /* Now 'export' those items to a new, smaller, graph while removing */
    size_t new_count = 0;
    for (size_t i = 0; i < n; i++) {
        if (items_to_keep[i] != 0) {
            new_items[new_count] = items[i];
            new_count++;
        } else {
            item_free(items[i]);
        }
    }
    free(items);

    size_t keep_i = 0, keep_j = 0;
    for (size_t i = 0; i < n; i++) {
        if (items_to_keep[i]) {
            for (size_t j = 0; j < n; j++) {
                if (items_to_keep[j]) {
                    new_adj_mat[keep_i][keep_j] = adj_matrix[i][j] == UINT8_MAX;
                    keep_j++;
                }
            }
            keep_i++;
        }
        keep_j = 0;
    }

    free_matrix(&adj_matrix, n);

    free(items_to_keep);
    struct graph_of_items *new_dag = init_graph();
    new_dag->adj_matrix = new_adj_mat;
    new_dag->count = new_size;
    new_dag->item_list = new_items;
    return new_dag;
}

struct graph_of_items *
graph_get_subgraph_to_item(struct graph_of_items **super_graph,
                           sitem_id target_id) {
    assert(super_graph);
    assert(target_id >= 0);

    assert((*super_graph)->item_list);
    assert((*super_graph)->adj_matrix);

    size_t target_index = item_array_find(
        (const item *const *)(*super_graph)->item_list, target_id);

    struct graph_of_items *g = get_ancestor_dag(*super_graph, target_index);
    free(*super_graph); /* Free and nullify the original graph */
    *super_graph = NULL;
    return g;

}

void graph_print_dag_with_item_fields(const struct graph_of_items *dag,
                                      sitem_id target, uint64_t print_flags) {
    // TODO: See breakdown of git algorithm
    // Find sources (nodes with corresponding columns of all 0s)
    printf("Item %d is blocked by the following items:\n", target);

    // TODO: Extract magic number
    size_t dag_sources_indices[64] = {0};

    for (size_t i = 0; i < dag->count; i++)
        item_print_fancy(dag->item_list[i], print_flags, NULL);
}
