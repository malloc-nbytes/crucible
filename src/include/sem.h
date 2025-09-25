#ifndef SEM_H_INCLUDED
#define SEM_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "ds/smap.h"

#include <forge/array.h>

typedef struct sym {
        const char *id;
        type *ty;
        int stack_offset;
        int extern_;
} sym;

typedef struct {
        smap_array scope;

        struct {
                type *type;
                int inproc;
                int rsp;
        } proc;

        str_array errs;
        int stack_offset;

        void *loop;
} symtbl;

symtbl sem_analysis(program *p);

#endif // SEM_H_INCLUDED
