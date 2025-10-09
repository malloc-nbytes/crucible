#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

char *
int_to_cstr(int i)
{
        int digits = 0;
        if (i < 10) digits = 1;
        if (i < 100) digits = 2;
        if (i < 1000) digits = 3;
        if (i < 10000) digits = 4;
        if (i < 100000) digits = 5;
        if (i < 1000000) digits = 6;
        if (i < 10000000) digits = 7;
        if (i < 100000000) digits = 8;
        if (i < 1000000000) digits = 9;
        else digits = 10;

        char *s = (char *)malloc(digits + 1);
        sprintf(s, "%d", i);
        s[digits-1] = 0;
        return s;
}
