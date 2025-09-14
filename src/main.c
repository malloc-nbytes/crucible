#include "lexer.h"
#include "parser.h"
#include "sem.h"

#include <forge/arg.h>
#include <forge/err.h>
#include <forge/io.h>
#include <forge/utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
usage(void)
{
        printf("Usage: crucible [options..] <filepath>\n");
        exit(0);
}

int
main(int argc, char **argv)
{

        forge_arg *arg = forge_arg_alloc(argc, argv, 1);
        forge_arg *it = arg;
        const char *filepath = NULL;

        while (it) {
                if (!arg->h) {
                        if (filepath) {
                                forge_err("only 1 file is supported right now");
                        }
                        filepath = strdup(arg->s);
                } else if (arg->h == 1) {
                        forge_err_wargs("unknown option `%s`", arg->s);
                } else {
                        forge_err_wargs("unknown option `%s`", arg->s);
                }
                it = it->n;
        }
        forge_arg_free(arg);

        if (!filepath) { usage(); }

        char *src = forge_io_read_file_to_cstr(filepath);
        lexer l = lexer_create(src, filepath);
        //lexer_dump(&l);

        program p = parser_create_program(&l);
        //parser_dump_program(&p);

        symtbl tbl = sem_analysis(&p);
        NOOP(tbl);

        return 0;
}
