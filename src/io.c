#include "io.h"
#include "loc.h"
#include "global.h"

#include <forge/array.h>
#include <forge/io.h>
#include <forge/cstr.h>

#include <stdio.h>
#include <stdlib.h>

char *
read_file_from_searchpaths(char **fp, const loc *loc)
{
        char *s = forge_io_read_file_to_cstr(*fp);
        if (s) return s;

        for (size_t i = 0; i < g_config.search_paths.len; ++i) {
                char *path = forge_cstr_builder(g_config.search_paths.data[i], "/", *fp, NULL);
                s = forge_io_read_file_to_cstr(path);
                if (s) {
                        *fp = path;
                        break;
                }
                free(path);
        }

        if (!s) {
                fprintf(stderr, "%scould not find file `%s`\n", (loc ? loc_err(*loc) : ""), *fp);
                for (size_t i = 0; i < g_config.search_paths.len; ++i) {
                        if (i == 0)
                                fprintf(stderr, "out of the following paths:\n");
                        fprintf(stderr, "    %s\n", g_config.search_paths.data[i]);
                }
                exit(1);
        }

        return s;
}
