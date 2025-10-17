#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <forge/array.h>

#include <stdint.h>

extern struct {
        uint32_t flags;
        char *filepath;
        char *outname;
        str_array search_paths;
        str_array lib_search_paths;
        str_array link_libs;
} g_config;

#endif // GLOBAL_H_INCLUDED
