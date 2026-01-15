#ifndef KWD_H_INCLUDED
#define KWD_H_INCLUDED

#define KWD_IF     "if"
#define KWD_ELSE   "else"
#define KWD_LET    "let"
#define KWD_WHILE  "while"
#define KWD_FOR    "for"
#define KWD_PROC   "proc"
#define KWD_RETURN "return"

#define KWD_CPL {                               \
                KWD_IF,                         \
                KWD_ELSE,                       \
                KWD_LET,                        \
                KWD_WHILE,                      \
                KWD_FOR,                        \
                KWD_PROC,                       \
                KWD_RETURN,                     \
        }

#define TY_I8   "i8"
#define TY_I16  "i16"
#define TY_I32  "i32"
#define TY_I64  "i64"
#define TY_U8   "u8"
#define TY_U16  "u16"
#define TY_U32  "u32"
#define TY_U64  "u64"
#define TY_VOID "void"

#define TY_CPL {                                \
        TY_I8,                                  \
        TY_I16,                                 \
        TY_I32,                                 \
        TY_I64,                                 \
        TY_U8,                                  \
        TY_U16,                                 \
        TY_U32,                                 \
        TY_U64,                                 \
        TY_VOID,                                \
    }

int iskwd(const char *s);
int isty(const char *s);

#endif // KWD_H_INCLUDED
