#ifndef ERR_H_INCLUDED
#define ERR_H_INCLUDED

#include "loc.h"

typedef struct {
        char *msg;
        loc loc;
} err;

err err_create(char *msg, loc loc);
void err_print(err err);

#endif // ERR_H_INCLUDED
