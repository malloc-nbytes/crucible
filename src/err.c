#include "err.h"

#include <stdio.h>

err
err_create(char *msg, loc loc)
{
        return (err) {
                .msg = msg,
                .loc = loc,
        };
}

void
err_print(err err)
{
        fprintf(stderr, "%s%s\n", loc_err(err.loc), err.msg);
}
