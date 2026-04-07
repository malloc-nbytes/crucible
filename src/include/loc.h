#ifndef LOC_H_INCLUDED
#define LOC_H_INCLUDED

#include <stddef.h>

typedef struct {
        size_t r;
        size_t c;
        const char *path;
} loc;

loc loc_from(size_t      r,
             size_t      c,
             const char *path);

const char *loc_cstr(loc l);

#endif // LOC_H_INCLUDED
