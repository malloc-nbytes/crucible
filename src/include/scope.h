#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include "symbol.h"

typedef struct scope {
        struct scope *parent;
        symmap symbols;
        symset locals;
} scope;

scope *scope_push(scope *parent);
symbol *scope_lookup(scope *, const char *name);
int scope_insert(scope *, symbol *);

#endif // SCOPE_H_INCLUDED
