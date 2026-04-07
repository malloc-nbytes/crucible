#include "ds/sv.h"
#include "err.h"

#include <assert.h>
#include <string.h>
#include <errno.h>

sv
sv_from(const char *s, ssize_t len)
{
        return (sv) {
                .s = s,
                .len = len == -1 ? strlen(s) : (size_t)len,
        };
}

#define BUF_CAP 512
const char *
sv_cstr(sv sv)
{
        assert(sv.len <= BUF_CAP);

        static char buf[BUF_CAP+1];

        memset(buf, 0, sizeof(buf));
        if (!strncpy(buf, sv.s, sv.len))
                fatal("failed to copy string: %s", strerror(errno));

        return buf;
}
#undef BUF_CAP
