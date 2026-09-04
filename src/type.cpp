#include "type.hpp"

type_void *g_type_void = NULL;
type_i32 *g_type_i32 = NULL;
type_u8 *g_type_u8 = NULL;

void
init_type_translation_unit(void)
{
        g_type_void = new type_void {
                .base = {
                        .k = TYPE_KIND_VOID,
                },
        };

        g_type_i32 = new type_i32 {
                .base = {
                        .k = TYPE_KIND_VOID,
                },
        };

        g_type_u8 = new type_u8 {
                .base = {
                        .k = TYPE_KIND_VOID,
                },
        };
}

int
is_type(std::string &s)
{
        static const char *types[] = TYPE_CPL;

        for (size_t i = 0; i < sizeof(types)/sizeof(*types); ++i) {
                if (s == types[i])
                        return 1;
        }

        return 0;
}

type_void *
type_void_alloc(void)
{
        return g_type_void;
}

type_i32 *
type_i32_alloc(void)
{
        return g_type_i32;
}

type_u8 *
type_u8_alloc(void)
{
        return g_type_u8;
}

type_ptr *
type_ptr_alloc(type *inner)
{
        return new type_ptr {
                .base = {
                        .k = TYPE_KIND_PTR,
                },
                .inner = inner,
        };
}
