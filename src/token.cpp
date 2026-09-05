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
        return new token {
                .lx  = std::string(st, len),
                .k   = k,
                .loc = location_from(r, c, std::string(path)),
        };
}

const char *
token_kind_to_cstring(token_kind k)
{
        switch (k) {
        case TOKEN_KIND_EOF:                    return "EOF";
        case TOKEN_KIND_KEYWORD:                return "Keyword";
        case TOKEN_KIND_TYPE:                   return "Type";
        case TOKEN_KIND_IDENTIFIER:             return "Identifier";
        case TOKEN_KIND_INTEGER_LITERAL:        return "Integer Literal";
        case TOKEN_KIND_STRING_LITERAL:         return "String Literal";
        case TOKEN_KIND_L_PAREN:                return "(";
        case TOKEN_KIND_R_PAREN:                return ")";
        case TOKEN_KIND_L_CURLY:                return "{";
        case TOKEN_KIND_R_CURLY:                return "}";
        case TOKEN_KIND_L_SQR:                  return "[";
        case TOKEN_KIND_R_SQR:                  return "]";
        case TOKEN_KIND_COMMA:                  return ",";
        case TOKEN_KIND_COLON:                  return ":";
        case TOKEN_KIND_SEMICOLON:              return ";";
        case TOKEN_KIND_PIPE:                   return "|";
        case TOKEN_KIND_UPTICK:                 return "^";
        case TOKEN_KIND_AMPERSAND:              return "&";
        case TOKEN_KIND_ASTERISK:               return "*";
        case TOKEN_KIND_PLUS:                   return "+";
        case TOKEN_KIND_HYPHEN:                 return "-";
        case TOKEN_KIND_FORWARD_SLASH:          return "/";
        case TOKEN_KIND_BANG:                   return "!";
        case TOKEN_KIND_PERCENT:                return "%";
        case TOKEN_KIND_EQUALS:                 return "=";
        case TOKEN_KIND_GREATERTHAN:            return ">";
        case TOKEN_KIND_LESSTHAN:               return "<";
        case TOKEN_KIND_PERIOD:                 return ".";
        case TOKEN_KIND_TILDE:                  return "~";
        case TOKEN_KIND_PLUS_EQUALS:            return "+=";
        case TOKEN_KIND_MINUS_EQUALS:           return "-=";
        case TOKEN_KIND_ASTERISK_EQUALS:        return "*=";
        case TOKEN_KIND_FORWARD_SLASH_EQUALS:   return "/=";
        case TOKEN_KIND_PERCENT_EQUALS:         return "%=";
        case TOKEN_KIND_BANG_EQUALS:            return "!=";
        case TOKEN_KIND_GREATERTHAN_EQUALS:     return ">=";
        case TOKEN_KIND_LESSTHAN_EQUALS:        return "<=";
        case TOKEN_KIND_DOUBLE_EQUALS:          return "==";
        case TOKEN_KIND_DOUBLE_AMPERSAND:       return "&&";
        case TOKEN_KIND_DOUBLE_PIPE:            return "||";
        case TOKEN_KIND_UPTICK_EQUALS:          return "^=";
        case TOKEN_KIND_AMPERSAND_EQUALS:       return "&=";
        case TOKEN_KIND_PIPE_EQUALS:            return "|=";
        case TOKEN_KIND_TILDE_EQUALS:           return "~=";
        case TOKEN_KIND_ELIPSIS:                return "...";
        default:                                return "UNKNOWN";
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
