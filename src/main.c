#include "flags.h"
#include "lexer.h"
#include "parser.h"
#include "sem.h"
#include "asm.h"
#include "visitor.h"

#include <forge/arg.h>
#include <forge/err.h>
#include <forge/io.h>
#include <forge/utils.h>
#include <forge/chooser.h>
#include <forge/str.h>
#include <forge/cstr.h>
#include <forge/cmd.h>
#include <forge/array.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct {
        uint32_t flags;
        char *filepath;
        char *outname;
        str_array search_paths;
        str_array lib_search_paths;
        str_array link_libs;
} g_config = {
        .flags = 0x0000,
        .filepath = NULL,
        .outname = NULL,
        .search_paths = dyn_array_empty(str_array),
        .lib_search_paths = dyn_array_empty(str_array),
        .link_libs = dyn_array_empty(str_array),
};

void
usage(void)
{
        printf("Usage: cruc [options..] <filepath>\n");
        printf("Options:\n");
        printf("    --%s, -%c    view this help information\n", FLAG_2HY_HELP, FLAG_1HY_HELP);
        printf("    --%s, -%c    set the output filename\n", FLAG_2HY_OUTPUT, FLAG_1HY_OUTPUT);
        printf("    --%s, -%c <dir>   add directory to library search path\n", FLAG_2HY_LIBPATH, FLAG_1HY_LIBPATH);
        printf("    --%s, -%c <name>  link with library lib<name>.so or .a\n", FLAG_2HY_LIB, FLAG_1HY_LIB);
        exit(0);
}

static void
link(str_array obj_filepaths)
{
        int (*_cmd)(const char *) = (g_config.flags & FLAG_TYPE_VERBOSE) == 0
                ? cmd_s
                : cmd;

        forge_str ld = forge_str_from("ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc ");
        // append -L paths
        FOREACH(path, g_config.lib_search_paths.data, g_config.lib_search_paths.len, {
                forge_str_concat(&ld, "-L");
                forge_str_concat(&ld, path);
                forge_str_concat(&ld, " ");
        });
        FOREACH(path, g_config.lib_search_paths.data, g_config.lib_search_paths.len, {
                forge_str_concat(&ld, "-rpath=");
                forge_str_concat(&ld, path);
                forge_str_concat(&ld, " ");
        });
        // append -l libs
        FOREACH(lib, g_config.link_libs.data, g_config.link_libs.len, {
                forge_str_concat(&ld, "-l");
                forge_str_concat(&ld, lib);
                forge_str_concat(&ld, " ");
        });
        forge_str_concat(&ld, "-o ");
        forge_str_concat(&ld, g_config.outname);

        str_array found = dyn_array_empty(str_array);

        for (size_t i = 0; i < obj_filepaths.len; ++i) {
                int ok = 1;
                for (size_t j = 0; j < found.len; ++j) {
                        if (!strcmp(obj_filepaths.data[i], found.data[j])) {
                                ok = 0;
                                break;
                        }
                }
                if (ok) {
                        dyn_array_append(found, obj_filepaths.data[i]);
                }
        }

        FOREACH(obj, found.data, found.len, {
                forge_str_concat(&ld, " ");
                forge_str_concat(&ld, obj);
        });

        _cmd(ld.data);

        FOREACH(obj, found.data, found.len, {
                _cmd(forge_cstr_builder("rm ", obj, NULL));
        });

        forge_str_destroy(&ld);
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
                        } else if (it->s[0] == FLAG_1HY_SEARCHPATH) {
                                if (!it->n) { forge_err_wargs("option -%c requires an argument", FLAG_1HY_SEARCHPATH); }
                                it = it->n;
                                dyn_array_append(g_config.search_paths, strdup(it->s));
                        } else if (it->s[0] == FLAG_1HY_LIBPATH) {
                                if (!it->n) { forge_err_wargs("option -%c requires an argument", FLAG_1HY_LIBPATH); }
                                it = it->n;
                                dyn_array_append(g_config.lib_search_paths, strdup(it->s));
                        } else if (it->s[0] == FLAG_1HY_LIB) {
                                if (!it->n) { forge_err_wargs("option -%c requires an argument", FLAG_1HY_LIB); }
                                it = it->n;
                                dyn_array_append(g_config.link_libs, strdup(it->s));
                        } else if (it->s[0] == FLAG_1HY_VERBOSE) {
                                g_config.flags |= FLAG_TYPE_VERBOSE;
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
                        } else if (!strcmp(it->s, FLAG_2HY_NOSTD)) {
                                g_config.flags |= FLAG_TYPE_NOSTD;
                        } else if (!strcmp(it->s, FLAG_2HY_LIBPATH)) {
                                if (!it->n) { forge_err_wargs("option --%s requires an argument", FLAG_2HY_LIBPATH); }
                                it = it->n;
                                dyn_array_append(g_config.lib_search_paths, strdup(it->s));
                        } else if (!strcmp(it->s, FLAG_2HY_LIB)) {
                                if (!it->n) { forge_err_wargs("option --%s requires an argument", FLAG_2HY_LIB); }
                                it = it->n;
                                dyn_array_append(g_config.link_libs, strdup(it->s));
                        } else if (!strcmp(it->s, FLAG_2HY_VERBOSE)) {
                                g_config.flags |= FLAG_TYPE_VERBOSE;
                        }
                        else {
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
        if (!src) {
                forge_err_wargs("could not read filepath `%s`", g_config.filepath);
        }

        lexer    l      = lexer_create(src, g_config.filepath);
        program  *p     = parser_create_program(&l);
        symtbl  *tbl    = sem_analysis(p);

        link(asm_gen(p, tbl));

        return 0;
}
