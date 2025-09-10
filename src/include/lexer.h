#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stddef.h>

typedef enum {
        TOKEN_TYPE_EOF,
        TOKEN_TYPE_IDENTIFIER,
        TOKEN_TYPE_KEYWORD,
        TOKEN_TYPE_INTEGER_LITERAL,
        TOKEN_TYPE_STRING_LITERAL,
        TOKEN_TYPE_CHARACTER_LITERAL,

        // Symbols
        TOKEN_TYPE_LEFT_PARENTHESIS,
        TOKEN_TYPE_RIGHT_PARENTHESIS,
        TOKEN_TYPE_LEFT_CURLY,
        TOKEN_TYPE_RIGHT_CURLY,
        TOKEN_TYPE_LEFT_SQUARE,
        TOKEN_TYPE_RIGHT_SQUARE,
        TOKEN_TYPE_BACKTICK,
        TOKEN_TYPE_TILDE,
        TOKEN_TYPE_BANG,
        TOKEN_TYPE_AT,
        TOKEN_TYPE_HASH,
        TOKEN_TYPE_DOLLAR,
        TOKEN_TYPE_PERCENT,
        TOKEN_TYPE_UPTICK,
        TOKEN_TYPE_AMPERSAND,
        TOKEN_TYPE_ASTERISK,
        TOKEN_TYPE_PLUS,
        TOKEN_TYPE_MINUS,
        TOKEN_TYPE_EQUALS,
        TOKEN_TYPE_PIPE,
        TOKEN_TYPE_BACKSLASH,
        TOKEN_TYPE_FORWARDSLASH,
        TOKEN_TYPE_LESSTHAN,
        TOKEN_TYPE_GREATERTHAN,
        TOKEN_TYPE_COMMA,
        TOKEN_TYPE_PERIOD,
        TOKEN_TYPE_QUESTION,
        TOKEN_TYPE_SEMICOLON,
        TOKEN_TYPE_COLON,

        TOKEN_TYPE_PLUS_EQUALS,
        TOKEN_TYPE_MINUS_EQUALS,
        TOKEN_TYPE_ASTERISK_EQUALS,
        TOKEN_TYPE_FORWARDSLASH_EQUALS,
        TOKEN_TYPE_PERCENT_EQUALS,
        TOKEN_TYPE_AMPERSAND_EQUALS,
        TOKEN_TYPE_PIPE_EQUALS,
        TOKEN_TYPE_UPTICK_EQUALS,

        TOKEN_TYPE_DOUBLE_EQUALS,
        TOKEN_TYPE_GREATERTHAN_EQUALS,
        TOKEN_TYPE_LESSTHAN_EQUALS,
        TOKEN_TYPE_BANG_EQUALS,
        TOKEN_TYPE_DOUBLE_AMPERSAND,
        TOKEN_TYPE_DOUBLE_PIPE
} token_type;

typedef struct token {
        char *lx;
        token_type ty;
        size_t r;
        size_t c;
        const char *fp;
        struct token *next;
} token;

typedef struct {
        token *hd;
        token *tl;
} lexer;

lexer lexer_create(const char *src, const char *fp);
void lexer_dump(const lexer *l);

#endif // LEXER_H_INCLUDED
