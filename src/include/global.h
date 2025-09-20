#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <stdint.h>

extern struct {
        uint32_t flags;
        char *filepath;
        char *outname;
} g_config;

#endif // GLOBAL_H_INCLUDED
