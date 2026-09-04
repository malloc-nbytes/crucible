#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include <string>

#define TYPE_I32 "i32"
#define TYPE_U8 "u8"
#define TYPE_VOID "void"

#define TYPE_CPL { \
        TYPE_I32, \
        TYPE_U8, \
        TYPE_VOID, \
}

typedef enum {
        TYPE_KIND_VOID = 0,
        TYPE_KIND_I32,
        TYPE_KIND_U8,
        TYPE_KIND_PTR,
} type_kind;

typedef struct { type_kind k; } type;

int is_type(std::string &s);

#endif // TYPE_H_INCLUDED
