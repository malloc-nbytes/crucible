#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "token.h"
#include "ds/sv.h"

#include <stddef.h>

typedef struct {
        size_t    pos;
        tokenp_ar tokens;
        sv        src;
        sv        path;
} lexer;

void init_lexer_translation_unit(void);

lexer lexer_lex_file(const char *path,
                     const char *src);

#endif // LEXER_H_INCLUDED
