#ifndef STR_H_INCLUDED
#define STR_H_INCLUDED

#include "array.h"

#include <stddef.h>

typedef struct {
        char   *chars;
        size_t  len;
        size_t  cap;
} str;

ARRAY_DEFINE(str, str_ar);

str         str_create(void);
str         str_from(const char *chars);
const char *str_cstr(const str *s);
void        str_append(str *s, char c);
void        str_concat(str *s, const char *chars);
void        str_overwrite(str *s, const char *repl);
void        str_clear(str *s);
size_t      str_len(const str *s);
void        str_destroy(str *s);
void        str_insert(str *s, size_t i, char ch);
void        str_cut(str *s, size_t i);
void        str_rm(str *s, size_t i);
char        str_pop(str *s);
char        str_at(const str *s, size_t i);
char        str_pop(str *s);
void        str_trim_before(str *s);
str         str_from_fmt(const char *fmt, ...);
void        str_remove_range(str *s, size_t start, size_t count);

#endif // STR_H_INCLUDED
