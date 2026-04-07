#ifndef KW_H_INCLUDED
#define KW_H_INCLUDED

#define KW_LET    "let"
#define KW_IF     "if"
#define KW_ELSE   "else"
#define KW_PROC   "proc"
#define KW_EXPORT "export"
#define KW_EXTERN "extern"

#define KW_CPL { \
        KW_LET,  \
        KW_IF,   \
        KW_ELSE, \
        KW_PROC, \
        KW_EXPORT, \
        KW_EXTERN, \
}

int kw_iskw(const char *s);

#endif // KW_H_INCLUDED
