#include "parser.hpp"

parser
parse(lexer *l)
{
        parser p = (parser) {
                .l = l,
        };

        return p;
}
