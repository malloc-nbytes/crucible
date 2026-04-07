#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "ds/array.h"

typedef enum {
        TYPE_KIND_VOID = 0,
        TYPE_KIND_I32,
        TYPE_KIND_U8,
        TYPE_KIND_U32,
        TYPE_KIND_NUMBER, // literal
        TYPE_KIND_PTR,
} type_kind;

typedef struct { type_kind kind; } type;

ARRAY_DEFINE(type *, typep_ar);

typedef struct { type base; } type_void;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_u8;
typedef struct { type base; } type_u32;
typedef struct { type base; } type_number;

typedef struct {
        type  base;
        type *to;
} type_ptr;

// Call init_types_translation_unit() to init.
extern type_void   *g_type_void;
extern type_i32    *g_type_i32;
extern type_u8     *g_type_u8;
extern type_u32    *g_type_u32;
extern type_number *g_type_number;
extern typep_ar     g_types;

void       init_types_translation_unit(void);
type_void *type_void_alloc(void); // Should not need to be used; index g_types instead.
type_i32  *type_i32_alloc(void);  // Should not need to be used; index g_types instead.
type_u8   *type_u8_alloc(void);   // Should not need to be used; index g_types instead.
type_u32  *type_u32_alloc(void);  // Should not need to be used; index g_types instead.
type_ptr  *type_ptr_alloc(type *to);

#endif // TYPES_H_INCLUDED
