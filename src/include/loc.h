#ifndef LOC_H_INCLUDED
#define LOC_H_INCLUDED

#include <stddef.h>

typedef struct {
        const char *fp;
        size_t r;
        size_t c;
} loc;

#endif // LOC_H_INCLUDED
