#include "kwd.h"

#include <string.h>

int
iskwd(const char *s)
{
        const char *kwds[] = KWD_CPL;
        for (size_t i = 0; i < sizeof(kwds)/sizeof(*kwds); ++i) {
                if (!strcmp(s, kwds[i]))
                        return 1;
        }
        return 0;
}
