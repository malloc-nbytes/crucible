#ifndef SEM_H_INCLUDED
#define SEM_H_INCLUDED

#include "types.h"
#include "parser.h"
#include "ds/smap.h"

#include <forge/array.h>

typedef struct {
        const char *id;
        const type *ty;
} sem_var;

DYN_ARRAY_TYPE(sem_var *, sem_var_array);

typedef struct {
        const char *id;
        sem_var_array params;
        const type *type;
} sem_proc;

DYN_ARRAY_TYPE(sem_proc *, sem_proc_array);

typedef struct {
        smap_array scope;
        sem_proc_array procs;
} symtbl;

symtbl sem_analysis(program *p);

#endif // SEM_H_INCLUDED
