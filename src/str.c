#include "ds/str.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static void
try_resize(str *s)
{
        if (s->len >= s->cap) {
                s->cap = s->cap ? s->cap*2 : 2;
                s->chars = (char *)realloc(s->chars, s->cap);
                memset(s->chars + s->len+1, 0, s->cap - s->len);
        }
}

str
str_create(void)
{
        str s = (str) {
                .chars = calloc(2, 1),
                .len   = 0,
                .cap   = 2,
        };

        return s;
}

str
str_from(const char *chars)
{
        str s = str_create();
        str_concat(&s, chars);
        return s;
}

inline const char *
str_cstr(const str *s)
{
        return s->chars;
}

void
str_append(str *s, char c)
{
        try_resize(s);
        s->chars[s->len++] = c;
}

void
str_concat(str *s, const char *chars)
{
        if (!chars || !*chars)
                return;

        for (size_t i = 0; chars[i]; ++i)
                str_append(s, chars[i]);
}

void
str_clear(str *s)
{
        memset(s->chars, 0, s->cap);
        s->len = 0;
}

void
str_overwrite(str *s, const char *repl)
{
        str_clear(s);
        str_concat(s, repl);
}

inline size_t
str_len(const str *s)
{
        return s->len;
}

void
str_destroy(str *s)
{
        if (s->chars)
                free(s->chars);
        s->chars = NULL;
        s->len   = 0;
        s->cap   = 0;
}

void
str_insert(str    *s,
           size_t  i,
           char    ch)
{
        if (i > s->len)
                i = s->len;

        try_resize(s);

        memmove(s->chars+i+1,
                s->chars+i,
                s->len-i+1);

        s->chars[i] = ch;

        s->len++;
}

void
str_cut(str *s, size_t i)
{
        if (i >= s->len)
                return;

        memset(s->chars+i, 0, s->cap-i);
        s->len = i;
}

void
str_rm(str *s, size_t i)
{
        if (i >= s->len)
                return;

        memmove(s->chars+i,
                s->chars+i+1,
                s->len-i);

        s->chars[s->len-1] = 0;
        --s->len;
}

char
str_at(const str *s, size_t i)
{
        assert(i < s->len);

        return s->chars[i];
}

char
str_pop(str *s)
{
        char ch;

        if (s->len <= 0)
                return 0;

        ch = s->chars[s->len-1];
        str_rm(s, s->len-1);

        return ch;
}

void
str_trim_before(str *s)
{
        while (s->len > 0 && s->chars[0] == ' ')
                str_rm(s, 0);
}

str
str_from_fmt(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);

        va_list ap_copy;
        va_copy(ap_copy, ap);
        int needed = vsnprintf(NULL, 0, fmt, ap_copy);
        va_end(ap_copy);

        if (needed < 0) {
                va_end(ap);
                fprintf(stderr, "str_from_fmt: vsnprintf failed\n");
                exit(EXIT_FAILURE);
        }

        str s;
        s.len = (size_t)needed;
        s.cap = s.len + 1;
        s.chars = malloc(s.cap);

        if (!s.chars) {
                va_end(ap);
                fprintf(stderr, "str_from_fmt: malloc failed\n");
                exit(EXIT_FAILURE);
        }

        vsnprintf(s.chars, s.cap, fmt, ap);
        va_end(ap);

        return s;
}

void
str_remove_range(str *s, size_t start, size_t count)
{
      if (count == 0 || start >= s->len)
          return;

      if (start + count > s->len)
          count = s->len - start;

      memmove(s->chars + start,
              s->chars + start + count,
              s->len - (start + count) + 1); // +1 for null

      s->len -= count;

      if (s->len < s->cap)
          s->chars[s->len] = 0;
}
