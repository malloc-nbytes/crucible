#ifndef SEM_H_INCLUDED
#define SEM_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "ds/smap.h"

#include <forge/array.h>

typedef struct {
        const char *id;
        type *ty;
} sym;

typedef struct {
        smap_array scope;

        struct {
                type *type;
                int inproc;
        } proc;

        str_array errs;
} symtbl;

symtbl sem_analysis(program *p);

#endif // SEM_H_INCLUDED
