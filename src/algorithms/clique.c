#include "clique.h"

// Begin Clique Callback Functions

// End Clique Callback Functions
// Begin Helper Functions

/**
 * @brief This function returns the vertex with the highest degree.
 * Ties are broken by choosing the vertex with the higher index.
 *
 * @param row_idx
 * @param col_idx
 * @param degrees
 * @return int
 */
int _compare_degrees(vertex u, vertex v, int* degrees) {
    int degree_u = degrees[u];
    int degree_v = degrees[v];

    if (degree_u == degree_v) {
        return u > v ? u : v;
    }

    return degree_u > degree_v ? u : v;
}

int _compare_vertex_id(vertex u, vertex v, int* _unused) {
    // This param is necessary to match the expected function
    // signature for the graph_make_directed function, but it is
    // causing an unused variable warning. This assertion is here to
    // silence the warning :)
    assert(_unused == NULL || _unused != NULL);
    return max(u, v);
}

// End Helper Functions
// Begin Specialized Clique Functions (k=1,2,3)

void enumerate_three_cliques(Graph* graph, void* collection, void (*record)(void*, vertex, vertex, vertex)) {
    assert(graph != NULL);
    assert(graph->is_directed == false);
    assert(graph->adjacency_matrix != NULL);
    assert(graph->adjacency_matrix->is_set);

    // Generate a degree oriented graph
    int* undirected_degrees = graph_get_out_degrees(graph);
    Graph* directed_graph = graph_make_directed(graph, _compare_degrees, undirected_degrees);

    // Store these variables for easy access
    CompressedSparseRow* adjacency_matrix = directed_graph->adjacency_matrix;
    int* ptr_rows = adjacency_matrix->ptr_rows;
    int* idx_cols = adjacency_matrix->idx_cols;

    // Loop over the (filtered) out-edges of v
    for (vertex v = 0; v < graph->num_vertices; v++) {
        int idx_v_begin_read = ptr_rows[v];
        int idx_v_end_read = ptr_rows[v + 1];

        for (int idx_u_nnz = idx_v_begin_read; idx_u_nnz < idx_v_end_read; idx_u_nnz++) {
            vertex u = idx_cols[idx_u_nnz];

            for (int idx_w_nnz = idx_u_nnz + 1; idx_w_nnz < idx_v_end_read; idx_w_nnz++) {
                vertex w = idx_cols[idx_w_nnz];

                if (graph_get_edge(graph, u, w) > 0) {
                    record(collection, u, v, w);
                }
            }

            for (int idx_w_nnz = idx_u_nnz + 1; idx_w_nnz < idx_v_end_read; idx_w_nnz++) {
                vertex w = idx_cols[idx_w_nnz];

                if (graph_get_edge(graph, u, w) > 0) {
                    record(collection, u, v, w);
                }
            }
        }
    }

    free(undirected_degrees);
    graph_delete(&directed_graph);
}

void enumerate_four_cliques(Graph* graph, void* collection, void (*record)(void*, vertex, vertex, vertex, vertex)) {
    assert(graph != NULL);
    assert(graph->is_directed == false);
    assert(graph->adjacency_matrix != NULL);
    assert(graph->adjacency_matrix->is_set);

    // Generate a vertex id oriented graph
    Graph* directed_graph = graph_make_directed(graph, _compare_vertex_id, NULL);

    int* degrees = graph_get_in_degrees(directed_graph);

    // Store the triangles found in the graph
    vertex* triangle_ends = calloc(graph->num_vertices + 1, sizeof(vertex));

    // Store these variables for easy access
    CompressedSparseRow* adjacency_matrix = directed_graph->adjacency_matrix;
    int* ptr_rows = adjacency_matrix->ptr_rows;
    int* idx_cols = adjacency_matrix->idx_cols;

    // Loop over all vertices
    for (vertex u = 0; u < directed_graph->num_vertices; u++) {
        int idx_u_begin_read = ptr_rows[u];
        int idx_u_end_read = ptr_rows[u + 1];

        // Loop over neighbors of u
        for (int idx_nnz = idx_u_begin_read; idx_nnz < idx_u_end_read; idx_nnz++) {
            vertex v1 = idx_cols[idx_nnz];
            int count = 0;
            // loop over another out-neighbor v2 of u, that is "ahead" of v1 in list of out-neighbors
            // in other words, for each neighbor v1, look ahead to other neighbors v2
            // note, the graph is directed such that the id of v1 is less than the id of v2
            // this step looks for triangles formed by u and neighbors of u
            for (int idx_nnz_ahead = idx_nnz + 1; idx_nnz_ahead < idx_u_end_read; idx_nnz_ahead++) {
                vertex v2 = idx_cols[idx_nnz_ahead];

                // check if there is an edge between v1 and v2
                // since there is an edge between u, v1 and u, v2, this means there is a triangle
                // formed by u, v1, and v2
                if (graph_get_edge(graph, v1, v2) > 0) {
                    record(collection, u, v1, v2, -1);
                    triangle_ends[count] = v2;
                    count++;
                }
            }

            // loop over all triangles formed by u, v1
            for (int idx_ref_nnz = 0; idx_ref_nnz < count; idx_ref_nnz++) {
                vertex v2 = triangle_ends[idx_ref_nnz];
                vertex degree_v2 = degrees[v2];
                vertex remaining = count - idx_ref_nnz;

                int search_lower_bound = idx_ref_nnz + 1;

                if (degree_v2 >= remaining) {
                    for (int idx_ref_nnz_ahead = search_lower_bound; idx_ref_nnz_ahead < count; idx_ref_nnz_ahead++) {
                        vertex v3 = triangle_ends[idx_ref_nnz_ahead];
                        if (graph_get_edge(graph, v2, v3) > 0) {
                            record(collection, u, v1, v2, v3);
                        }
                    }
                } else {
                    for (int idx_v2_nnz = ptr_rows[v2]; idx_v2_nnz < ptr_rows[v2 + 1]; idx_v2_nnz++) {
                        vertex v3 = idx_cols[idx_v2_nnz];

                        if (search_lower_bound > count - 1) {
                            break;
                        }

                        if (v3 == u || v3 == v1) {
                            break;
                        }

                        if (array_binary_search_range(triangle_ends, count, search_lower_bound, count - 1, v3) >= 0) {
                            record(collection, u, v1, v2, v3);
                        }
                    }
                }
            }
        }
    }

    graph_delete(&directed_graph);
    free(triangle_ends);
}

// End Specialized Clique Functions (k=1,2,3,4)
// Begin Generalized Clique Functions (k>4)

/**
 * @brief This function is the recursive part of Chibi-Nishizeki's
 * algorithm for finding k-cliques in a graph.
 *
 * @param graph
 * @param target_k
 * @param current_k
 * @param current_vertex
 * @param current_clique
 * @param candidates
 * @param cliques
 */
void _find_neighbors(Graph* graph, int target_k, int current_k, int current_vertex, OrderedSet* current_clique, OrderedSet* candidates, CliqueSet* cliques) {
    assert(current_vertex >= 0);
    assert(current_vertex < graph->num_vertices);

    // If the current clique is of size k, add it to the list of cliques
    if (current_k == target_k) {
        clique clique_copy = calloc(target_k, sizeof(int));
        memcpy(clique_copy, current_clique->elements, target_k * sizeof(int));
        clique_set_insert(cliques, clique_copy);
        return;
    }

    OrderedSet* neighbors = graph_get_neighbors(graph, current_vertex);
    OrderedSet* new_candidates = ordered_set_intersection(candidates, neighbors);

    for (int i = 0; i < new_candidates->size; i++) {
        vertex candidate = new_candidates->elements[i];
        if (candidate > current_vertex) {
            ordered_set_insert(current_clique, candidate);
            _find_neighbors(graph, target_k, current_k + 1, candidate, current_clique, new_candidates, cliques);
            ordered_set_remove(current_clique, candidate);
        }
    }

    ordered_set_delete(&neighbors);
    ordered_set_delete(&new_candidates);
}

/**
 * @brief This function returns the set of all k-cliques in the graph.
 *
 * When k <= 3, this function uses specific functions to find each
 * k-clique. when k >= 4, this function uses Chibi-Nishizeki to find
 * each k-clique. The k-core of the input graph is computed to reduce
 * the search space of Chibi-Nishizeki.
 *
 * @param graph The graph to search.
 * @param k The size of the cliques to find.
 * @return CliqueSet* Clique set containing an nxk 2D array of
 * cliques, where n is the number of k-cliques found.
 */
CliqueSet* enumerate_k_cliques(Graph* graph, int k) {
    assert(graph != NULL);
    assert(graph->is_directed == false);
    assert(graph->adjacency_matrix != NULL);
    assert(graph->adjacency_matrix->is_set);

    assert(k > 4);

    // Use chiba nishizeki to find all k-cliques when k >= 4.
    // Create the resulting set of k-cliques
    int resize_value = 5;
    CliqueSet* cliques = clique_set_new(k, resize_value);

    // Compute and k-core to reduce search space
    bool* is_vertex_removed = get_vertices_not_in_k_core(graph, k);

    // Create clique of size k to compute k-cliques
    OrderedSet* current_clique = ordered_set_new(k);

    for (vertex v = 0; v < graph->num_vertices; v++) {
        if (is_vertex_removed[v] == true) {
            continue;
        }

        ordered_set_insert(current_clique, v);
        OrderedSet* candidates = graph_get_neighbors(graph, v);
        _find_neighbors(graph, k, 1, v, current_clique, candidates, cliques);
        ordered_set_remove(current_clique, v);
        ordered_set_delete(&candidates);
    }

    ordered_set_delete(&current_clique);
    free(is_vertex_removed);

    return cliques;
}

// End Generalized Clique Functions
