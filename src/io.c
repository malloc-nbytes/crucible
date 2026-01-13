#include "io.h"

#include <stdlib.h>
#include <stdio.h>

char *
load_file(const char *path)
{
        FILE   *f;
        char   *buf;
        size_t  size;

        if ((f = fopen(path, "rb")) == NULL) {
                perror("fopen");
                exit(1);
        }

        fseek(f, 0, SEEK_END);
        size = ftell(f);

        fseek(f, 0, SEEK_SET);
        buf = (char *)malloc(size + 1);

        if (fread(buf, 1, size, f) != size) {
                perror("fread");
                exit(1);
        }

        fclose(f);

        buf[size] = '\0';

        return buf;
}
