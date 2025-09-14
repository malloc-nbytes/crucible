#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

typedef enum {
        TYPE_KIND_I8 = 0,
        TYPE_KIND_I16,
        TYPE_KIND_I32,
        TYPE_KIND_I64,

        TYPE_KIND_U8,
        TYPE_KIND_U16,
        TYPE_KIND_U32,
        TYPE_KIND_U64,

        TYPE_KIND_PTR,
        TYPE_KIND_VOID,
        TYPE_KIND_NORETURN,
} type_kind;

typedef struct { type_kind kind; } type;

typedef struct { type base; } type_i8;
typedef struct { type base; } type_i16;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_i64;

typedef struct { type base; } type_u8;
typedef struct { type base; } type_u16;
typedef struct { type base; } type_u32;
typedef struct { type base; } type_u64;

typedef struct {
        type base;
        type *to;
} type_ptr;

typedef struct { type base; } type_void;
typedef struct { type base; } type_noreturn;

type_i32 *type_i32_alloc(void);
type_u32 *type_u32_alloc(void);
type_u8 *type_u8_alloc(void);
type_noreturn *type_noreturn_alloc(void);

#endif // TYPES_H_INCLUDED
