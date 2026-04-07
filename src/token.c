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
        case TK_PLUS: return "TK_PLUS";
        case TK_MINUS: return "TK_MINUS";
        case TK_FORWARDSLASH: return "TK_FORWARDSLASH";
        case TK_PERCENT: return "TK_PERCENT";
        case TK_ASTERISK: return "TK_ASTERISK";
	case TK_EQUALS: return "TK_EQUALS";
	case TK_PLUS_EQUALS: return "TK_PLUS_EQUALS";
	case TK_MINUS_EQUALS: return "TK_MINUS_EQUALS";
	case TK_FORWARDSLASH_EQUALS: return "TK_FORWARDSLASH_EQUALS";
	case TK_ASTERISK_EQUALS: return "TK_ASTERISK_EQUALS";
	case TK_AMPERSAND: return "TK_AMPERSAND";
	case TK_PIPE: return "TK_PIPE";
	case TK_DOUBLE_AMPERSAND: return "TK_DOUBLE_AMPERSAND";
	case TK_DOUBLE_PIPE: return "TK_DOUBLE_PIPE";
	case TK_AMPERSAND_EQUALS: return "TK_AMPERSAND_EQUALS";
	case TK_PIPE_EQUALS: return "TK_PIPE_EQUALS";
	case TK_UPTICK: return "TK_UPTICK";
	case TK_UPTICK_EQUALS: return "TK_UPTICK_EQUALS";
	case TK_GREATERTHAN: return "TK_GREATERTHAN";
	case TK_LESSTHAN: return "TK_LESSTHAN";
	case TK_GREATERTHAN_EQUALS: return "TK_GREATERTHAN_EQUALS";
	case TK_LESSTHAN_EQUALS: return "TK_LESSTHAN_EQUALS";
        default: return "UNKNOWN";
        }

        return NULL;
}
