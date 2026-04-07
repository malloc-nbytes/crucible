#include "lexer.h"
#include "io.h"

#include <stdio.h>

int
main(void)
{
        init_lexer_translation_unit();

        const char *path;
        char       *src;
        lexer       l;

        path = "input.cr";
        src  = load_file(path);
        l    = lexer_lex_file(path, src);

        lexer_dump(&l);

        return 0;
}
