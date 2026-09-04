#include "lex.hpp"
#include "err.hpp"

#include <cassert>
#include <cstring>
#include <unordered_map>

std::unordered_map<std::string, token_kind> g_opmap = {};

void
init_lexer_translation_unit(void)
{
        g_opmap.clear();

        g_opmap.emplace("(", TOKEN_KIND_L_PAREN);
        g_opmap.emplace(")", TOKEN_KIND_R_PAREN);
        g_opmap.emplace("{", TOKEN_KIND_L_CURLY);
        g_opmap.emplace("}", TOKEN_KIND_R_CURLY);
        g_opmap.emplace("[", TOKEN_KIND_L_SQR);
        g_opmap.emplace("]", TOKEN_KIND_R_SQR);
        g_opmap.emplace(",", TOKEN_KIND_COMMA);
        g_opmap.emplace(":", TOKEN_KIND_COLON);
        g_opmap.emplace(";", TOKEN_KIND_SEMICOLON);
        g_opmap.emplace("|", TOKEN_KIND_PIPE);
        g_opmap.emplace("^", TOKEN_KIND_UPTICK);
        g_opmap.emplace("&", TOKEN_KIND_AMPERSAND);
        g_opmap.emplace("*", TOKEN_KIND_ASTERISK);
        g_opmap.emplace("+", TOKEN_KIND_PLUS);
        g_opmap.emplace("-", TOKEN_KIND_HYPHEN);
        g_opmap.emplace("/", TOKEN_KIND_FORWARD_SLASH);
        g_opmap.emplace("!", TOKEN_KIND_BANG);
        g_opmap.emplace("%", TOKEN_KIND_PERCENT);
        g_opmap.emplace("=", TOKEN_KIND_EQUALS);
        g_opmap.emplace(">", TOKEN_KIND_GREATERTHAN);
        g_opmap.emplace("<", TOKEN_KIND_LESSTHAN);
        g_opmap.emplace(".", TOKEN_KIND_PERIOD);
        g_opmap.emplace("+=", TOKEN_KIND_PLUS_EQUALS);
        g_opmap.emplace("-=", TOKEN_KIND_MINUS_EQUALS);
        g_opmap.emplace("*=", TOKEN_KIND_ASTERISK_EQUALS);
        g_opmap.emplace("/=", TOKEN_KIND_FORWARD_SLASH_EQUALS);
        g_opmap.emplace("!=", TOKEN_KIND_BANG_EQUALS);
        g_opmap.emplace(">=", TOKEN_KIND_GREATERTHAN_EQUALS);
        g_opmap.emplace("<=", TOKEN_KIND_LESSTHAN_EQUALS);
        g_opmap.emplace("==", TOKEN_KIND_DOUBLE_EQUALS);
        g_opmap.emplace("&&", TOKEN_KIND_DOUBLE_AMPERSAND);
        g_opmap.emplace("||", TOKEN_KIND_DOUBLE_PIPE);
        g_opmap.emplace("...", TOKEN_KIND_ELIPSIS);
}

template <typename Callable>
static size_t
consume_while(const char *s, Callable pred)
{
        size_t i;
        for (i = 0; s[i] && pred(s[i]); ++i);
        return i;
}

static token_kind
determine_op(const char *s, size_t *len)
{
        char buf[32] = {};

        if (*len >= sizeof(buf))
                *len = sizeof(buf) - 1;

        memcpy(buf, s, *len);
        while (*len > 0) {
                auto it = g_opmap.find(buf);
                if (it != g_opmap.end())
                        return it->second;

                buf[--(*len)] = 0;
        }

        return TOKEN_KIND_EOF;
}

void
lexer_dump(const lexer *l)
{
        for (const auto &t : l->ts)
                token_dump(t);
}

lexer
lex_file(const char     *path,
         char           *src)
{
        lexer l = lexer {
                .ts     = std::vector<token *>(),
                .path   = std::string(path),
                .src    = std::string(src),
                .cursor = 0,
        };

        size_t i = 0, c = 1, r = 1;

        while (i < l.src.length()) {
                char ch = src[i];

                if (ch == '\n') {
                        i += 1;
                        r += 1;
                        c = 1;
                } else if (isspace(ch)) {
                        i += 1;
                        c += 1;
                } else if (ch == '_' || isalpha(ch)) {
                        size_t len = consume_while(src+i, [](int c) {
                                return isalnum(c) || c == '_';
                        });
                        l.ts.push_back(token_alloc(src+i, len, TOKEN_KIND_IDENTIFIER, r, c, path));
                        i += len;
                        c += len;
                } else if (isdigit(ch)) {
                        size_t len = consume_while(src+i, [](int c) {
                                return isalnum(c);
                        });
                        l.ts.push_back(token_alloc(src+i, len, TOKEN_KIND_INTEGER_LITERAL, r, c, path));
                        i += len;
                        c += len;
                } else if (ch == '"') {
                        size_t len = consume_while(src+i, [](int c) {
                                return c != '"';
                        });
                        l.ts.push_back(token_alloc(src+i+1, len, TOKEN_KIND_STRING_LITERAL, r, c, path));
                        i += len+2;
                        c += len+2;
                } else {
                        size_t len = consume_while(src+i, [](int c) {
                                return !isalnum(c) && !isspace(c) && c != '_';
                        });
                        token_kind op = determine_op(src+i, &len);
                        if (op == TOKEN_KIND_EOF)
                                fatal("unknown operator near `%c'", ch);
                        l.ts.push_back(token_alloc(src+i, len, op, r, c, path));
                        i += len;
                        c += len;
                }
        }

        l.ts.push_back(token_alloc("EOF", 3, TOKEN_KIND_EOF, r, c, path));

        return l;
}
