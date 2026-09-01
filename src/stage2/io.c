#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *
load_file(const char *path)
{
        FILE   *f;
        char   *buf;
        long int  size;

        if ((f = fopen(path, "rb")) == NULL)
                return NULL;

        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        buf = (char *)malloc((size_t)(size + 1));
        size_t _ = fread(buf, 1, (size_t)size, f);
        (void)_;

        fclose(f);

        buf[size] = '\0';

        return buf;
}
