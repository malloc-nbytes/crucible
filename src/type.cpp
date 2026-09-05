#include "type.hpp"

#include <format>
#include <utility>

type_void       *g_type_void = NULL;
type_i32        *g_type_i32  = NULL;
type_u8         *g_type_u8   = NULL;

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

std::string
type_to_string(const type *const t)
{
        switch (t->k) {
        case TYPE_KIND_VOID: return "void";
        case TYPE_KIND_I32:  return "i32";
        case TYPE_KIND_U8:   return "u8";
        case TYPE_KIND_PTR:
                return format("Ptr(%s)", type_to_string(((type_ptr *)t)->inner));
        }
        std::unreachable();
}

int
type_is_assignment(token_kind k)
{
        return k == TOKEN_KIND_EQUALS || (type_compound_assignment(k) != std::nullopt);
}

std::optional<token_kind>
type_compound_assignment(token_kind k)
{
        switch (k) {
        case TOKEN_KIND_PLUS_EQUALS:          return TOKEN_KIND_PLUS;
        case TOKEN_KIND_MINUS_EQUALS:         return TOKEN_KIND_HYPHEN;
        case TOKEN_KIND_ASTERISK_EQUALS:      return TOKEN_KIND_ASTERISK;
        case TOKEN_KIND_FORWARD_SLASH_EQUALS: return TOKEN_KIND_FORWARD_SLASH;
        case TOKEN_KIND_PERCENT_EQUALS:       return TOKEN_KIND_PERCENT;
        case TOKEN_KIND_PIPE_EQUALS:          return TOKEN_KIND_PIPE;
        case TOKEN_KIND_AMPERSAND_EQUALS:     return TOKEN_KIND_AMPERSAND;
        case TOKEN_KIND_UPTICK_EQUALS:        return TOKEN_KIND_UPTICK;
        default: return std::nullopt;
        }
        std::unreachable();
}

int
type_check(const type *const t0,
           const type *const t1)
{
        if (t0->k != t1->k)
                return 0;
        if (t0->k == TYPE_KIND_PTR)
                return type_check(((type_ptr *)t0)->inner, ((type_ptr *)t1)->inner);
        return 1;
}
