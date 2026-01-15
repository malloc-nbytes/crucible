#include "scope.h"
#include "utils.h"

scope *
scope_push(scope *parent)
{
        NOOP(parent);
        TODO("");
        return NULL;
}

symbol *
scope_lookup(scope      *sc,
             const char *name)
{
        NOOP(sc, name);
        TODO("");
        return NULL;
}

int
scope_insert(scope *sc, symbol *sym)
{
        NOOP(sc, sym);
        TODO("");
        return 0;
}
