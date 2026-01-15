#ifndef SET_H_INCLUDED
#define SET_H_INCLUDED

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define SET_DEFAULT_CAPACITY 256

#define SET_DECL(type, setname) \
        typedef unsigned (*setname##_hash_sig)(type *); \
        typedef int      (*setname##_cmp_sig)(type *, type *); \
        typedef void     (*setname##_vfree_sig)(type *); \
        \
        typedef struct __##setname##_node { \
                type v; \
                struct __##setname##_node *n; \
        } __##setname##_node; \
        \
        typedef struct setname { \
                struct { \
                        __##setname##_node **data; \
                        size_t len; \
                        size_t cap; \
                        size_t sz; \
                } tbl; \
                setname##_hash_sig hash; \
                setname##_cmp_sig cmp; /*returns 0 on equal*/   \
                setname##_vfree_sig vfree; \
        } setname; \
        \
        setname setname##_create(setname##_hash_sig hash, setname##_cmp_sig cmp, setname##_vfree_sig vfree); \
        void    setname##_insert(setname *s, type v); \
        void    setname##_remove(setname *s, type v); \
        int     setname##_contains(const setname *s, type v); \
        void    setname##_destroy(setname *s); \
        void    setname##_print(const setname *s, void (*show)(type *t)); \
        size_t  setname##_size(const setname *s); \
        type  **setname##_iter(const setname *s);

#define SET_IMPL(type, setname) \
        typedef unsigned (*setname##_hash_sig)(type *); \
        typedef int      (*setname##_cmp_sig)(type *, type *); \
        typedef void     (*setname##_vfree_sig)(type *); \
        setname \
        setname##_create(setname##_hash_sig hash, \
                         setname##_cmp_sig cmp, \
                         setname##_vfree_sig vfree) \
        { \
                assert(hash); \
                assert(cmp); \
                __##setname##_node **data \
                        = (__##setname##_node **)calloc(SET_DEFAULT_CAPACITY, sizeof(__##setname##_node *)); \
                return (setname) { \
                        .tbl = { \
                                .data = data, \
                                .len  = 0, \
                                .cap  = SET_DEFAULT_CAPACITY, \
                                .sz   = 0, \
                        }, \
                        .hash = hash, \
                        .cmp = cmp, \
                        .vfree = vfree, \
                }; \
        } \
        void \
        setname##_insert(setname *s, type v) \
        { \
                unsigned idx = s->hash(&v) % s->tbl.cap; \
                __##setname##_node *it = s->tbl.data[idx]; \
                __##setname##_node *prev = NULL; \
                while (it) { \
                        if (s->cmp(&it->v, &v) == 0) return; \
                        prev = it; \
                        it = it->n; \
                } \
                __##setname##_node *n = (__##setname##_node *)malloc(sizeof(__##setname##_node)); \
                n->v = v; \
                n->n = NULL; \
                if (prev) { \
                        prev->n = n; \
                } else { \
                        s->tbl.data[idx] = n; \
                        ++s->tbl.len; \
                } \
                ++s->tbl.sz; \
        } \
        \
        void \
        setname##_remove(setname *s, type v) \
        { \
                unsigned idx = s->hash(&v) % s->tbl.cap; \
                __##setname##_node *it = s->tbl.data[idx]; \
                __##setname##_node *prev = NULL; \
                while (it) { \
                        if (s->cmp(&it->v, &v) == 0) { \
                                if (prev) { \
                                        prev->n = it->n; \
                                        /* TODO: free 'it' */ \
                                } else { \
                                        prev = it->n; \
                                        s->tbl.data[idx] = prev; \
                                        /* TODO: free 'it' */ \
                                } \
                                --s->tbl.sz; \
                        } \
                        prev = it; \
                        it = it->n; \
                } \
        } \
        \
        int \
        setname##_contains(const setname *s, type v) \
        { \
                unsigned idx = s->hash(&v) % s->tbl.cap; \
                __##setname##_node *it = s->tbl.data[idx]; \
                while (it) { \
                        if (s->cmp(&it->v, &v) == 0) return 1; \
                        it = it->n; \
                } \
                return 0; \
        } \
        \
        void \
        setname##_destroy(setname *s) \
        { \
                for (size_t i = 0; i < s->tbl.cap; ++i) { \
                        __##setname##_node *it = s->tbl.data[i]; \
                        while (it) { \
                                if (s->vfree) s->vfree(&it->v); \
                                __##setname##_node *tmp = it; \
                                it = it->n; \
                                free(tmp); \
                        } \
                } \
        } \
        \
        void \
        setname##_print(const setname *s, \
                        void (*show)(type *t)) \
        { \
                for (size_t i = 0; i < s->tbl.cap; ++i) { \
                        __##setname##_node *it = s->tbl.data[i]; \
                        while (it) { \
                                show(&it->v); \
                                it = it->n; \
                        } \
                } \
        } \
        \
        size_t \
        setname##_size(const setname *s) \
        { \
                return s->tbl.sz; \
        } \
        \
        type ** \
        setname##_iter(const setname *s) \
        { \
                type **ar = (type **)malloc(sizeof(type *) * s->tbl.sz + 1); \
                size_t ar_n = 0; \
                for (size_t i = 0; i < s->tbl.cap; ++i) { \
                        __##setname##_node *it = s->tbl.data[i]; \
                        while (it) { \
                                ar[ar_n++] = &it->v; \
                                it = it->n; \
                        } \
                } \
                ar[ar_n] = NULL; \
                return ar; \
        } \

#endif // SET_H_INCLUDED
