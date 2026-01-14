#include "cstr.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

char *
format(const char *fmt, ...)
{
        va_list ap;
        va_list ap_copy;
        char *result = NULL;
        int len;

        if (fmt == NULL)
                return NULL;

        va_start(ap, fmt);
        va_copy(ap_copy, ap);
        len = vsnprintf(NULL, 0, fmt, ap_copy);
        va_end(ap_copy);

        if (len < 0) {
                va_end(ap);
                return NULL;
        }

        result = malloc((size_t)len + 1);
        if (result == NULL) {
                va_end(ap);
                return NULL;
        }

        len = vsnprintf(result, (size_t)len + 1, fmt, ap);
        va_end(ap);

        if (len < 0) {
                free(result);
                return NULL;
        }

        return result;
}
