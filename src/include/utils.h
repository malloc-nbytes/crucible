#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define _NOOP_1(x) ((void)(x))
#define _NOOP_2(x, y) ((void)(x), (void)(y))
#define _NOOP_3(x, y, z) ((void)(x), (void)(y), (void)(z))
#define _NOOP_4(x, y, z, w) ((void)(x), (void)(y), (void)(z), (void)(w))
#define _NOOP_5(x, y, z, w, v) ((void)(x), (void)(y), (void)(z), (void)(w), (void)(v))
#define _NOOP_6(x, y, z, w, v, u) ((void)(x), (void)(y), (void)(z), (void)(w), (void)(v), (void)(u))
#define _NOOP_7(x, y, z, w, v, u, t) ((void)(x), (void)(y), (void)(z), (void)(w), (void)(v), (void)(u), (void)(t))
#define _NOOP_8(x, y, z, w, v, u, t, s) ((void)(x), (void)(y), (void)(z), (void)(w), (void)(v), (void)(u), (void)(t), (void)(s))

#define _GET_NOOP_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME

#define NOOP(...) _GET_NOOP_MACRO(__VA_ARGS__, _NOOP_8, _NOOP_7, _NOOP_6, _NOOP_5, _NOOP_4, _NOOP_3, _NOOP_2, _NOOP_1)(__VA_ARGS__)

// Perform a foreach loop on each element in `ar` with
// the named value of `k` until `len`, doing `blk`.
#define FOREACH(k, ar, len, blk)                                \
        for (size_t __iter = 0; __iter < len; ++__iter) {       \
                typeof((ar)[__iter]) k = (ar)[__iter];          \
                blk;                                            \
        }

#define TODO(msg)                                                       \
        do {                                                            \
                fprintf(stderr, "%s:%d: TODO: %s(): " msg "\n",         \
                        __FILE__, __LINE__, __FUNCTION__);              \
                exit(1);                                                \
        } while (0)

char *format(const char *fmt, ...);

#endif // UTILS_H_INCLUDED
