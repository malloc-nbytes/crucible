#include "token.hpp"

#include <iostream>
#include <string>

token *
token_alloc(const char *st,
            size_t      len,
            token_kind  k,
            size_t      r,
            size_t      c,
            const char *path)
{
        token *t;

        t      = new token;
        t->lx  = std::string(st, len);
        t->k   = k;
        t->loc = location_from(r, c, std::string(path));

        return t;
}

const char *
token_kind_to_cstring(token_kind k)
{
        switch (k) {
        case TOKEN_KIND_EOF: return "TOKEN_KIND_EOF";
        case TOKEN_KIND_KEYWORD: return "TOKEN_KIND_KEYWORD";
        case TOKEN_KIND_PRIMITIVE_TYPE: return "TOKEN_KIND_PRIMITIVE_TYPE";
        case TOKEN_KIND_IDENTIFIER: return "TOKEN_KIND_IDENTIFIER";
        case TOKEN_KIND_INTEGER_LITERAL: return "TOKEN_KIND_INTEGER_LITERAL";
        case TOKEN_KIND_STRING_LITERAL: return "TOKEN_KIND_STRING_LITERAL";
        case TOKEN_KIND_L_PAREN: return "TOKEN_KIND_L_PAREN";
        case TOKEN_KIND_R_PAREN: return "TOKEN_KIND_R_PAREN";
        case TOKEN_KIND_L_CURLY: return "TOKEN_KIND_L_CURLY";
        case TOKEN_KIND_R_CURLY: return "TOKEN_KIND_R_CURLY";
        case TOKEN_KIND_L_SQR: return "TOKEN_KIND_L_SQR";
        case TOKEN_KIND_R_SQR: return "TOKEN_KIND_R_SQR";
        case TOKEN_KIND_COMMA: return "TOKEN_KIND_COMMA";
        case TOKEN_KIND_COLON: return "TOKEN_KIND_COLON";
        case TOKEN_KIND_SEMICOLON: return "TOKEN_KIND_SEMICOLON";
        case TOKEN_KIND_PIPE: return "TOKEN_KIND_PIPE";
        case TOKEN_KIND_UPTICK: return "TOKEN_KIND_UPTICK";
        case TOKEN_KIND_AMPERSAND: return "TOKEN_KIND_AMPERSAND";
        case TOKEN_KIND_ASTERISK: return "TOKEN_KIND_ASTERISK";
        case TOKEN_KIND_PLUS: return "TOKEN_KIND_PLUS";
        case TOKEN_KIND_HYPHEN: return "TOKEN_KIND_HYPHEN";
        case TOKEN_KIND_FORWARD_SLASH: return "TOKEN_KIND_FORWARD_SLASH";
        case TOKEN_KIND_BANG: return "TOKEN_KIND_BANG";
        case TOKEN_KIND_PERCENT: return "TOKEN_KIND_PERCENT";
        case TOKEN_KIND_EQUALS: return "TOKEN_KIND_EQUALS";
        case TOKEN_KIND_GREATERTHAN: return "TOKEN_KIND_GREATERTHAN";
        case TOKEN_KIND_LESSTHAN: return "TOKEN_KIND_LESSTHAN";
        case TOKEN_KIND_PERIOD: return "TOKEN_KIND_PERIOD";
        case TOKEN_KIND_PLUS_EQUALS: return "TOKEN_KIND_PLUS_EQUALS";
        case TOKEN_KIND_MINUS_EQUALS: return "TOKEN_KIND_MINUS_EQUALS";
        case TOKEN_KIND_ASTERISK_EQUALS: return "TOKEN_KIND_ASTERISK_EQUALS";
        case TOKEN_KIND_FORWARD_SLASH_EQUALS: return "TOKEN_KIND_FORWARD_SLASH_EQUALS";
        case TOKEN_KIND_BANG_EQUALS: return "TOKEN_KIND_BANG_EQUALS";
        case TOKEN_KIND_GREATERTHAN_EQUALS: return "TOKEN_KIND_GREATERTHAN_EQUALS";
        case TOKEN_KIND_LESSTHAN_EQUALS: return "TOKEN_KIND_LESSTHAN_EQUALS";
        case TOKEN_KIND_DOUBLE_EQUALS: return "TOKEN_KIND_DOUBLE_EQUALS";
        case TOKEN_KIND_DOUBLE_AMPERSAND: return "TOKEN_KIND_DOUBLE_AMPERSAND";
        case TOKEN_KIND_DOUBLE_PIPE: return "TOKEN_KIND_DOUBLE_PIPE";
        case TOKEN_KIND_ELIPSIS: return "TOKEN_KIND_ELIPSIS";
        default: return "UNKNOWN";
        }
}

void
token_dump(const token *const &t)
{
        std::printf("<lx=%s, k=%s, loc=%s>\n",
                    t->lx.c_str(),
                    token_kind_to_cstring(t->k),
                    location_to_string(t->loc).c_str());
}
