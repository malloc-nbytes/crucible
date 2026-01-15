#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

#include "lex.h"
#include "type.h"
#include "ds/map.h"
#include "ds/set.h"

#include <stdint.h>

typedef struct scope scope;

typedef enum {
        SK_VAR = 0,
        SK_PARAM,
        SK_PROC,
        SK_TYPE,
} sym_kind;

typedef struct {
        sym_kind kind;
        const token *name;
        type *type;

        uint32_t id;
        scope *defined_in;
        int mutable;
} symbol;

MAP_DECL(char *, symbol *, symmap);
SET_DECL(symbol *, symset);

#endif // SYMBOL_H_INCLUDED
