#ifndef ARGUMENT_H_INCLUDED
#define ARGUMENT_H_INCLUDED

#include <stddef.h>

typedef struct argument {
        // Points to the start of the argument
        // after any hyphens (max 2).
        char *s;

        // The number of hyphens (max 2).
        size_t h;

        // Points to the character after the
        // first equals is encountered.
        char *eq;

        // The next argument
        struct argument *n;
} argument;

argument *argument_alloc(int argc, char **argv, int skip_first);

void argument_free(argument *arg);

#endif // ARGUMENT_H_INCLUDED
