#include "io.hpp"
#include "lex.hpp"
#include "parser.hpp"

#include <iostream>

int
main(void)
{
        const char *path;
        char       *src;

        path = "input.cr";
        src  = load_file(path);

        init_lexer_translation_unit();

        lexer l = lex_file(path, src);

        parser p = parse(&l);
        (void)p;

        return 0;
}
