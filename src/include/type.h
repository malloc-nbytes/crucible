#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

typedef enum {
        TYPE_KIND_VOID = 0,
        TYPE_KIND_I32,
} type_kind;

typedef struct {
        type_kind kind;
} type;

typedef struct { type base; } type_void;
typedef struct { type base; } type_i32;

#endif // TYPE_H_INCLUDED
