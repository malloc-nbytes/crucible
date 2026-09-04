#include "symbol.hpp"

symbol *
symbol_alloc(symbol_kind         k,
             int                 id,
             std::string         name,
             type               *ty,
             location            loc)
{
        symbol *sym;

        sym       = new symbol;
        sym->k    = k;
        sym->id   = id;
        sym->name = name;
        sym->ty   = ty;
        sym->loc  = loc;

        return sym;
}
