#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include "token.hpp"

#include <string>
#include <vector>

typedef struct {
        std::vector<token *>    ts;
        std::string             path;
        std::string             src;
} lexer;

void init_lexer_translation_unit(void);
lexer lex_file(const char *path, char *src);
void lexer_dump(const lexer *l);

#endif // LEX_H_INCLUDED
