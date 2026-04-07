#include "kw.h"

#include <stddef.h>
#include <string.h>

int
kw_iskw(const char *s)
{
        static const char *kwds[] = KW_CPL;

        for (size_t i = 0; i < sizeof(kwds)/sizeof(*kwds); ++i)
                if (!strcmp(s, kwds[i]))
                        return 1;

        return 0;
}
