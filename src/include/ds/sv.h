#ifndef SV_H_INCLUDED
#define SV_H_INCLUDED

#include <stddef.h>
#include <stdlib.h>

typedef struct {
        const char *s;
        size_t      len;
} sv;

// len=-1 for taking the length until '\0'.
sv sv_from(const char *s, ssize_t len);

const char *sv_cstr(sv sv);

#endif
