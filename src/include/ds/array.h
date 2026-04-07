#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include <stdlib.h>

#define ARRAY_DEFINE(ty, name) \
    typedef struct name {        \
        ty *data;                \
        size_t len, cap;         \
    } name

#define array_empty(arr_ty)                     \
        (arr_ty) {                              \
                .data = NULL,                   \
                        .len = 0,               \
                        .cap = 0,               \
                        }

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

#define array_free(da)       \
    do {                         \
        if ((da).data != NULL) { \
                free((da).data); \
        }                        \
        (da).len = (da).cap = 0; \
    } while (0)

#define array_at_s(da, i)                                      \
    ((i) < (da).len ? (da).data[i] : (fprintf(stderr,              \
    "[array error]: index %zu is out of bounds (len = %zu)\n", \
    (size_t)(i), (size_t)(da).len), exit(1), (da).data[0]))

#define array_at(da, i) ((da).data[i])

#define array_clear(da) (da).len = 0;

#define array_rm_at(da, idx) \
    do {                                                     \
        for (size_t __i_ = (idx); __i_ < (da).len-1; ++__i_) \
            (da).data[__i_] = (da).data[__i_+1];             \
        (da).len--;                                          \
    } while (0)

#define array_insert_at(da, idx, value) \
    do { \
        if ((idx) > (da).len) { \
            fprintf(stderr, \
                "[array error]: insert index %zu is out of bounds (len = %zu)\n", \
                (size_t)(idx), (size_t)(da).len); \
            exit(1); \
        } \
        if ((da).len >= (da).cap) { \
            (da).cap = (da).cap ? (da).cap * 2 : 2; \
            (da).data = (typeof(*((da).data)) *) \
                realloc((da).data, (da).cap * sizeof(*((da).data))); \
            if ((da).data == NULL) { \
                fprintf(stderr, "[array error]: realloc failed\n"); \
                exit(1); \
            } \
        } \
        for (size_t __i = (da).len; __i > (idx); --__i) { \
            (da).data[__i] = (da).data[__i - 1]; \
        } \
        (da).data[idx] = (value); \
        (da).len++; \
    } while (0)

// Common types for arrays.

ARRAY_DEFINE(int,      int_ar);
ARRAY_DEFINE(char,     char_ar);
ARRAY_DEFINE(char *,   cstr_ar);
ARRAY_DEFINE(size_t,   size_t_ar);
ARRAY_DEFINE(float,    float_ar);
ARRAY_DEFINE(double,   double_ar);
ARRAY_DEFINE(long,     long_ar);
ARRAY_DEFINE(unsigned, unsigned_ar);
ARRAY_DEFINE(void *,   voidp_ar);
ARRAY_DEFINE(const char *, ccstr_ar);

#endif // ARRAY_H_INCLUDED
