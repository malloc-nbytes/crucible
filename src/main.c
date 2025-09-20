#include "flags.h"
#include "lexer.h"
#include "parser.h"
#include "sem.h"
#include "asm.h"

#include <forge/arg.h>
#include <forge/err.h>
#include <forge/io.h>
#include <forge/utils.h>
#include <forge/cmd.h>
#include <forge/cstr.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct {
        uint32_t flags;
        char *filepath;
        char *outname;
} g_config = {
        .flags = 0x0000,
        .filepath = NULL,
        .outname = NULL,
};

void
usage(void)
{
        printf("Usage: cruc [options..] <filepath>\n");
        printf("Options:\n");
        printf("    --%s, -%c    view this help information\n", FLAG_2HY_HELP, FLAG_1HY_HELP);
        printf("    --%s, -%c    set the output filename\n", FLAG_2HY_OUTPUT, FLAG_1HY_OUTPUT);
        exit(0);
}

static void
assemble(void)
{
        char *nasm = forge_cstr_builder("nasm -f elf64 -g -F dwarf ", g_config.filepath, ".asm -o ",
                                        g_config.outname, ".o", NULL);
        char *ld = forge_cstr_builder("ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -o ",
                                      g_config.outname, " ",
                                      g_config.outname, ".o", NULL);

        char *rm_o = forge_cstr_builder("rm ", g_config.outname, ".o ", NULL);
        char *rm_asm = forge_cstr_builder("rm ", g_config.filepath, ".asm", NULL);

        cmd_s(nasm);
        cmd_s(ld);
        cmd_s(rm_o);

        if ((g_config.flags & FLAG_TYPE_ASM) == 0) {
                cmd_s(rm_asm);
        }

        free(nasm);
        free(ld);
        free(rm_o);
        free(rm_asm);
}

static void
handle_args(int argc, char **argv)
{
        forge_arg *arg = forge_arg_alloc(argc, argv, 1);
        forge_arg *it = arg;

        while (it) {
                if (!it->h) {
                        if (g_config.filepath) { forge_err("only 1 file is supported right now"); }
                        g_config.filepath = strdup(it->s);
                } else if (it->h == 1) {
                        if (it->s[0] == FLAG_1HY_HELP) {
                                usage();
                        } else if (it->s[0] == FLAG_1HY_OUTPUT) {
                                if (!it->n) { forge_err_wargs("option -%c requires an argument", FLAG_1HY_OUTPUT); }
                                it = it->n;
                                g_config.outname = strdup(it->s);
                        } else {
                                forge_err_wargs("unknown option `%s`", it->s);
                        }
                } else {
                        if (!strcmp(it->s, FLAG_2HY_HELP)) {
                                usage();
                        } else if (!strcmp(it->s, FLAG_2HY_OUTPUT)) {
                                if (!it->n) { forge_err_wargs("option --%s requires an argument", FLAG_2HY_OUTPUT); }
                                it = it->n;
                                g_config.outname = strdup(it->s);
                        } else if (!strcmp(it->s, FLAG_2HY_ASM)) {
                                g_config.flags |= FLAG_TYPE_ASM;
                        } else {
                                forge_err_wargs("unknown option `%s`", it->s);
                        }
                }

                it = it->n;
        }

        forge_arg_free(arg);
}

int
main(int argc, char **argv)
{
        handle_args(argc, argv);

        if (!g_config.filepath) {
                usage();
        }
        if (!g_config.outname) {
                g_config.outname = "a.out";
        }

        char *src = forge_io_read_file_to_cstr(g_config.filepath);
        lexer l = lexer_create(src, g_config.filepath);

        program p = parser_create_program(&l);

        symtbl tbl = sem_analysis(&p);
        if (tbl.errs.len > 0) {
                for (size_t i = 0; i < tbl.errs.len; ++i) {
                        fprintf(stderr, "%s\n", tbl.errs.data[i]);
                }
                exit(1);
        }

        asm_gen(&p, &tbl);

        assemble();

        return 0;
}
