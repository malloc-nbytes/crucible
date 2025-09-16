#ifndef SMAP_H_INCLUDED
#define SMAP_H_INCLUDED

#include <forge/array.h>

#include <stddef.h>

#define SMAP_DEFAULT_TBL_CAPACITY 256

typedef unsigned (*smap_hash_sig)(const char *);

typedef struct __smap_node {
        char *k;       // copies the string given
        void *v;       // does not copy
        struct __smap_node *n;
} __smap_node;

typedef struct {
        struct {
                __smap_node **entries;
                size_t len;
                size_t cap;
        } tbl;

        smap_hash_sig hash;
        size_t sz;
} smap;

DYN_ARRAY_TYPE(smap, smap_array);

smap smap_create(smap_hash_sig hash);
void smap_insert(smap *map, const char *k, void *v);
void *smap_get(const smap *map, const char *k);
int smap_has(const smap *map, const char *k);
size_t smap_size(const smap *map);
void smap_free(smap *map);

#endif // SMAP_H_INCLUDED
