#include "lex.h"
#include "io.h"
#include "mem.h"

lexer
lex_file(const char *path)
{
        char   *src;
        lexer   l;
        size_t  r;
        size_t  c;
        size_t  i;

        l = (lexer) {
                .hd = NULL,
                .tl = NULL,
                .a  = {0},
        };

        arena_init(&l.a, 256);

        src = load_file(path);
        r   = 1;
        c   = 1;
        i   = 0;

        while (src[i]) {
                char ch = src[i];
        }

        return l;
}
