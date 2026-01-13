#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdlib.h>

/* This file is used for making stack alloc'd dynamic
 * arrays where we dont need to use the Array<T> DS. */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new dynamic array type globally.
 * Note: Use array_init() to initialize
 *       any instances of it.
 * Example:
 *   array_type(int, Int_Array);
 *
 *   void f(Int_Array *arr);
 *
 *   int main(void) {
 *       Int_Array arr;
 *       return 0;
 *   }
 */
#define ARRAY_TYPE(ty, name) \
    typedef struct name {        \
        ty *data;                \
        size_t len, cap;         \
    } name

#define array_empty(arr_ty)                 \
        (arr_ty) {                              \
                .data = NULL,                   \
                .len = 0,                       \
                .cap = 0,                       \
        }

/**
 * Initializes a global array type. This is only
 * used if you use ARRAY_TYPE().
 * Example:
 *   array_type(int, Int_Array);
 *
 *   int main(void) {
 *       Int_Array arr;
 *       array_init_type(arr); // <- here
 *       return 0;
 *   }
 */
#define array_init_type(da)                 \
    do {                                        \
        (da).data = malloc(sizeof(*(da).data)); \
        (da).cap = 1;                           \
        (da).len = 0;                           \
    } while (0)

/**
 * Creates a new dynamic array on the stack.
 * Example:
 *   array(int, int_vector);
 */
#define array(ty, name)                                        \
    struct {                                                       \
        ty *data;                                                  \
        size_t len, cap;                                           \
    } (name) = { .data = (typeof(ty) *)malloc(sizeof(ty)), .len = 0, .cap = 1 };

/**
 * Append to a dynamic array.
 * Example:
 *   array(int, int_vector);
 *   for (int i = 0; i < 10; ++i)
 *     array_append(int_vector, i);
 */
#define array_append(da, value)                                     \
    do {                                                                \
        if ((da).len >= (da).cap) {                                     \
            (da).cap = (da).cap ? (da).cap * 2 : 2;                     \
            (da).data = (typeof(*((da).data)) *)                        \
                realloc((da).data,                                      \
                        (da).cap * sizeof(*((da).data)));               \
        }                                                               \
        (da).data[(da).len++] = (value);                                \
    } while (0)

/**
 * Free a dynamic array.
 * Example:
 *   array(int, int_vector);
 *   array_free(int_vector);
*/
#define array_free(da)       \
    do {                         \
        if ((da).data != NULL) { \
                free((da).data); \
        }                        \
        (da).len = (da).cap = 0; \
    } while (0)

/**
 * Get an element safely at an index.
 * Will panic if the element is out of bounds.
 * Example:
 *   array(int, int_vector);
 *   array_append(int_vector, i);
 *   printf("%d\n", array_at_s(int_vector));
 */
#define array_at_s(da, i)                                      \
    ((i) < (da).len ? (da).data[i] : (fprintf(stderr,              \
    "[array error]: index %zu is out of bounds (len = %zu)\n", \
    (size_t)(i), (size_t)(da).len), exit(1), (da).data[0]))

/**
 * Get an element at an index.
 * Example:
 *   array(int, int_vector);
 *   array_append(int_vector, i);
 *   printf("%d\n", array_at(int_vector));
 */
#define array_at(da, i) ((da).data[i])

/**
 * Clear a dynamic array.
 * Example:
 *   array(int, int_vector);
 *   array_append(int_vector, i);
 *   array_clear(int_vector);
 */
#define array_clear(da) (da).len = 0;

/**
 * Remove an element at index `idx`.
 * Example:
 *   array(int, int_vector);
 *   ...
 *   array_rm_at(int_vector, 0);
 *   array_rm_at(int_vector, 5);
 *   ...
 */
#define array_rm_at(da, idx) \
    do {                                                     \
        for (size_t __i_ = (idx); __i_ < (da).len-1; ++__i_) \
            (da).data[__i_] = (da).data[__i_+1];             \
        (da).len--;                                          \
    } while (0)

#define array_explode(da) (da).data, (da).len, (da).cap

#define array_explode_mem(da) &(da).data, &(da).len, &(da).cap

// Common types for arrays.

ARRAY_TYPE(int,      int_array);
ARRAY_TYPE(char,     char_array);
ARRAY_TYPE(char *,   str_array);
ARRAY_TYPE(size_t,   size_t_array);
ARRAY_TYPE(float,    float_array);
ARRAY_TYPE(double,   double_array);
ARRAY_TYPE(long,     long_array);
ARRAY_TYPE(unsigned, unsigned_array);
ARRAY_TYPE(void *,   void_ptr_array);
ARRAY_TYPE(const char *, const_str_array);

#ifdef __cplusplus
}
#endif

#endif // ARRAY_H_INCLUDED
