#include "lex.h"
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>

static void
usage(void)
{
        printf("Usage: cruc [options..] <filepath>\n");
        exit(0);
}

int
main(void)
{
        (void)usage;

        lexer l = lex_file("input.cr");
        parse(&l);

        return 0;
}
