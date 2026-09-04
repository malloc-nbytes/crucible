#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

#include "type.hpp"
#include "location.hpp"

#include <string>

typedef enum {
        SYMBOL_KIND_LOCAL = 0,
        SYMBOL_KIND_PARAM,
        SYMBOL_KIND_PROC,
} symbol_kind;

typedef struct {
        symbol_kind      k;
        int              id;
        std::string      name;
        type            *ty;
        location         loc;
} symbol;

symbol *symbol_alloc(symbol_kind         k,
                     int                 id,
                     std::string         name,
                     type               *ty,
                     location            loc);

#endif // SYMBOL_H_INCLUDED
