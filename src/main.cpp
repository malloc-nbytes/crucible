#include "io.hpp"
#include "lex.hpp"
#include "parser.hpp"
#include "resolver.hpp"
#include "type.hpp"

#include <iostream>

int
main(void)
{
        const char *path;
        char       *src;

        path = "input.cr";
        src  = load_file(path);

        init_lexer_translation_unit();
        init_type_translation_unit();

        lexer l = lex_file(path, src);
        //lexer_dump(&l);

        parser p = parse(&l);
        resolve_ast(&p);

        return 0;
}
