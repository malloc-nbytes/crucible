#ifndef STRV_H_INCLUDED
#define STRV_H_INCLUDED

#include <stddef.h>

typedef struct {
        const char *st;
        size_t len;
} strv;

strv strv_from(const char *st, size_t len);
int strv_cmp1(strv s0, strv s1);
int strv_cmp2(strv s0, const char *s1);

#endif // STRV_H_INCLUDED
