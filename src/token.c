#include "token.h"
#include "mem.h"

token *
token_from(token_kind  k,
           const char *st,
           size_t      n,
           size_t      r,
           size_t      c,
           const char *path)
{
        token *t;

        t       = (token *)alloc(sizeof(token));
        t->kind = k;
        t->lx   = sv_from(st, n);
        t->loc  = loc_from(r, c, path);

        return t;
}

const char *
token_kind_cstr(token_kind k)
{
        switch (k) {
        case TK_IDENT: return "TK_IDENT";
        case TK_STRLIT: return "TK_STRLIT";
        case TK_INTLIT: return "TK_INTLIT";
        case TK_KW: return "TK_KW";
        case TK_TYPE: return "TK_TYPE";
        case TK_LPAREN: return "TK_LPAREN";
        case TK_RPAREN: return "TK_RPAREN";
        case TK_LCURLY: return "TK_LCURLY";
        case TK_RCURLY: return "TK_RCURLY";
        case TK_LSQR: return "TK_LSQR";
        case TK_RSQR: return "TK_RSQR";
        case TK_COLON: return "TK_COLON";
        case TK_BANG: return "TK_BANG";
        case TK_SEMI: return "TK_SEMI";
        case TK_EOF: return "TK_EOF";
        default: return "UNKNOWN";
        }

        return NULL;
}
