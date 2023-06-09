#ifndef ARRAY_UTIL_H_INCLUDED
#define ARRAY_UTIL_H_INCLUDED

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../utilities/math.h"

// Create and Delete Functions
int* array_generate_sequence(int val_begin, int val_inc, int len_array);
int* array_generate_sequence_shuffled(int val_begin, int val_inc, int len_array);

// Search Functions (Linear)
int array_linear_search(int* array, int len_array, int val_target);
int array_linear_search_range(int* array, int len_array, int idx_begin, int idx_end, int val_target);
int array_linear_search_or_closest(int* array, int len_array, int val_target);
int array_linear_search_range_or_closest(int* array, int len_array, int idx_begin, int idx_end, int val_target);

// Search Functions (Binary)
int array_binary_search(int* array, int len_array, int val_target);
int array_binary_search_range(int* array, int len_array, int idx_begin, int idx_end, int val_target);
int array_binary_search_or_closest(int* array, int len_array, int val_target);
int array_binary_search_range_or_closest(int* array, int len_array, int idx_begin, int idx_end, int val_target);

// Maniuplation Functions
int* array_shuffle(int* array, int len_array, bool use_original_array);
void array_parallel_sort_2(int* array_1, int* array_2, int len_array_1, int len_array_2, bool is_ascending);

// Indexing Functions
int array_filtered_argmin(int* array, int len_array, bool* idx_filtered);

// Ordered Set Functions
int array_count_symmetric_difference(int* array_1, int* array_2, int len_array_1, int len_array_2);
void array_union(int* array_1, int* array_2, int len_array_1, int len_array_2, int** ptr_array_union, int* len_array_union);

// Utility Functions
bool array_is_equal(int* array_1, int* array_2, int len_array_1, int len_array_2);
void array_print(int* array, int len_array, bool should_print_newline);
void array_bool_print(bool* array, int len_array, bool should_print_newline);

// Helpers
int cmp_ints_asc(const void* a, const void* b);
int cmp_ints_dsc(const void* a, const void* b);

#endif