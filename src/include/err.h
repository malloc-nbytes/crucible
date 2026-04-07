#ifndef ERR_H_INCLUDED
#define ERR_H_INCLUDED

#include <stdio.h>

#define fatal(fmt, ...)                                                 \
        do {                                                            \
                fprintf(stderr, "fatal: " fmt "\n", ##__VA_ARGS__);     \
                exit(1);                                                \
        } while (0)

#endif // ERR_H_INCLUDED
