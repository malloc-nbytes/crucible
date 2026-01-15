#ifndef SEMANTIC_H_INCLUDED
#define SEMANTIC_H_INCLUDED

#include "scope.h"
#include "parse.h"

scope semantic_toplvl_pass1(parse_context *pctx);

#endif // SEMANTIC_H_INCLUDED
