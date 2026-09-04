#include "io.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/stat.h>

int
file_exists(const char *fp)
{
        FILE *f;

        if ((f = fopen(fp, "r")) != NULL) {
                fclose(f);
                return 1;
        }

        return 0;
}

int
create_file(const char *fp, int force_overwrite)
{
        FILE *f;

        if (!force_overwrite && file_exists(fp))
                return 1;

        if (!(f = fopen(fp, "w")))
                return 0;

        fclose(f);
        return 1;
}

int
write_file(const char *fp, const char *content)
{
        FILE *f;

        if (!(f = fopen(fp, "w")))
                return 0;

        (void)fprintf(f, "%s", content);
        fclose(f);
        return 1;
}

int
is_dir(const char *path)
{
        struct stat path_stat;

        if (lstat(path, &path_stat) != 0)
                return 0;

        return S_ISDIR(path_stat.st_mode);
}

char *
load_file(const char *path)
{
        FILE   *f;
        char     *buf;
        size_t   size;

        if ((f = fopen(path, "rb")) == NULL)
                return NULL;

        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        buf = (char *)malloc(size + 1);
        size_t _ = fread(buf, 1, size, f); (void)_;

        fclose(f);

        buf[size] = '\0';

        return buf;
}
