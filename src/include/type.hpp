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

typedef struct { type base; } type_void;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_u8;

typedef struct {
        type base;
        type *inner;
} type_ptr;

void init_type_translation_unit(void);

int is_type(std::string &s);

type_void *type_void_alloc(void);
type_i32 *type_i32_alloc(void);
type_u8 *type_u8_alloc(void);
type_ptr *type_ptr_alloc(type *inner);

#endif // TYPE_H_INCLUDED
