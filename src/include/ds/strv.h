#ifndef STRV_H_INCLUDED
#define STRV_H_INCLUDED

#include <stddef.h>

typedef struct {
        const char *st;
        size_t len;
} strv;

#endif // STRV_H_INCLUDED
