#ifndef ASM_H_INCLUDED
#define ASM_H_INCLUDED

#include "parser.h"
#include "sem.h"

#include <forge/array.h>

str_array asm_gen(program *p, symtbl *tbl);

#endif // ASM_H_INCLUDED
