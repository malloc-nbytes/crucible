#include <stdint.h>
#include <stdlib.h>

struct {
        uint32_t flags;
        char *source;
} g_glconf = {
        .flags = 0x0000,
        .source = NULL,
};
