#ifndef SEM_H_INCLUDED
#define SEM_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "ds/smap.h"

typedef struct {
        const char *id;
        const type *ty;
} sym;

typedef struct {
        smap_array scope;
} symtbl;

symtbl sem_analysis(program *p);

#endif // SEM_H_INCLUDED
