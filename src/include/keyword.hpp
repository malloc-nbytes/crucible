#ifndef KEYWORD_H_INCLUDED
#define KEYWORD_H_INCLUDED

#include <string>

#define KEYWORD_PROC    "proc"
#define KEYWORD_EXPORT  "export"
#define KEYWORD_EXTERN  "extern"
#define KEYWORD_LET     "let"
#define KEYWORD_IF      "if"
#define KEYWORD_ELSE    "else"

#define KEYWORD_CPL { \
        KEYWORD_PROC, \
        KEYWORD_EXPORT, \
        KEYWORD_EXTERN, \
        KEYWORD_LET, \
        KEYWORD_IF, \
        KEYWORD_ELSE, \
}

int is_keyword(const std::string &s);

#endif // KEYWORD_H_INCLUDED
