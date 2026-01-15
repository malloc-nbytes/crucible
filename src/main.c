#include "lex.h"
#include "parse.h"
#include "ast.h"
#include "argument.h"
#include "semantic.h"
#include "utils.h"
#include "glconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
usage(void)
{
        printf("Usage: cruc [options..] <filepath>\n");
        exit(0);
}

static int
handle_arg2(const char *s)
{
        if (!strcmp(s, OPT_2HY_SHOW_LEXER))
                g_glconf.flags |= FT_SHOW_LEXER;
        else if (!strcmp(s, OPT_2HY_SHOW_AST))
                g_glconf.flags |= FT_SHOW_AST;
        else
                return 0;
        return 1;
}

static void
args(int argc, char *argv[])
{
        if (argc <= 1)
               usage();

        argument *arghd;
        argument *it;
        char      errb[4092];

        arghd = argument_alloc(argc, argv, 1);
        it    = arghd;

        while (it) {
                char   *s  = it->s;
                size_t  h  = it->h;

                if (h == 1) {
                        sprintf(errb, "`-%s' one hyphen options are unimplemented", s);
                        goto bad;
                }

                if (h == 2 && !handle_arg2(s)) {
                        sprintf(errb, "unknown option `--%s'", s);
                        goto bad;
                }

                if (h == 0 && g_glconf.source) {
                        sprintf(errb, "at `%s', only one source file is required; already have `%s'",
                                s, g_glconf.source);
                        goto bad;
                }

                if (h == 0)
                        g_glconf.source = strdup(s);

                it = it->n;
        }

        argument_free(arghd);

        if (!g_glconf.source)
                usage();

        return;
 bad:
        argument_free(arghd);
        fprintf(stderr, "error(CLI): %s\n", errb);
        exit(1);
}

static void
cleanup(void)
{
        if (g_glconf.source)
                free(g_glconf.source);
}

int
main(int argc, char *argv[])
{
        args(argc, argv);

        lexer l = lex_file(g_glconf.source);
        parse_context parse_ctx = parse(&l);
        scope toplvl_scope = semantic_toplvl_pass1(&parse_ctx);
        NOOP(toplvl_scope);

        cleanup();

        return 0;
}
