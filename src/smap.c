#include "ds/smap.h"
#include "mem.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static __smap_node *
smap_node_alloc(const char *k,
                void       *v)
{
        __smap_node *n = (__smap_node *)alloc(sizeof(__smap_node));
        n->k = strdup(k);
        n->v = v;
        n->n = NULL;
        return n;
}

static void
smap_node_free(__smap_node *it)
{
        free(it->k);
        // TODO: what to do with value?
        free(it);
}

static unsigned
djb2(const char *s)
{
        unsigned hash = 5381;
        int c;

        while ((c = *s++))
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
}

smap
smap_create(smap_hash_sig hash)
{
        return (smap) {
                .tbl = {
                        .entries = (__smap_node **)calloc(
                                SMAP_DEFAULT_TBL_CAPACITY,
                                sizeof(__smap_node *)
                         ),
                        .cap = SMAP_DEFAULT_TBL_CAPACITY,
                        .len = 0,
                },
                .hash = hash ? hash : djb2,
                .sz = 0,
        };
}

void
smap_insert(smap       *map,
            const char *k,
            void       *v)
{
        assert(map && k && v);

        size_t idx = map->hash(k)%map->tbl.cap;
        __smap_node *it = map->tbl.entries[idx];
        __smap_node *prev = NULL;

        while (it) {
                if (!strcmp(k, it->k)) {
                        it->v = v;
                        return;
                }
                prev = it;
                it = it->n;
        }

        it = smap_node_alloc(k, v);

        if (prev) {
                prev->n = it;
        } else {
                map->tbl.entries[idx] = it;
                ++map->tbl.len;
        }

        ++map->sz;
}

void *
smap_get(const smap *map,
         const char *k)
{
        assert(map && k);

        size_t idx = map->hash(k)%map->tbl.cap;
        __smap_node *it = map->tbl.entries[idx];

        while (it) {
                if (!strcmp(k, it->k)) {
                        return it->v;
                }
                it = it->n;
        }

        return NULL;
}

int
smap_has(const smap *map,
         const char *k)
{
        assert(map && k);
        return smap_get(map, k) != NULL;
}

size_t
smap_size(const smap *map)
{
        assert(map);
        return map->sz;
}

void
smap_free(smap *map)
{
        assert(map);

        for (size_t i = 0; i < map->tbl.cap; ++i) {
                __smap_node *it = map->tbl.entries[i];
                while (it) {
                        __smap_node *tmp = it->n;
                        smap_node_free(it);
                        it = tmp;
                }
        }

        map->tbl.len = 0;
        map->sz = 0;
}
