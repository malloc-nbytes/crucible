#ifndef KWDS_H_INCLUDED
#define KWDS_H_INCLUDED

#define KWD_I8   "i8"
#define KWD_I16  "i16"
#define KWD_I32  "i32"
#define KWD_I64  "i64"
#define KWD_U8   "u8"
#define KWD_U16  "u16"
#define KWD_U32  "u32"
#define KWD_U64  "u64"
#define KWD_VOID "void"

#define KWD_PROC   "proc"
#define KWD_EXPORT "export"
#define KWD_LET    "let"
#define KWD_RETURN "return"
#define KWD_EXIT   "exit"
#define KWD_EXTERN "extern"

#define KWD_IF    "if"
#define KWD_WHILE "while"
#define KWD_FOR   "for"
#define KWD_ELSE  "else"

#define KWD_BREAK    "break"
#define KWD_CONTINUE "continue"

#define KWD_STRUCT "struct"

#define KWD_MODULE "module"
#define KWD_IMPORT "import"
#define KWD_WHERE  "where"

#define KWD_EMBED  "embed"

#define KWDS {                                  \
                KWD_I8,                         \
                KWD_I16,                        \
                KWD_I32,                        \
                KWD_I64,                        \
                KWD_U8,                         \
                KWD_U16,                        \
                KWD_U32,                        \
                KWD_U64,                        \
                KWD_VOID,                       \
                KWD_PROC,                       \
                KWD_LET,                        \
                KWD_IF,                         \
                KWD_WHILE,                      \
                KWD_FOR,                        \
                KWD_ELSE,                       \
                KWD_EXPORT,                     \
                KWD_RETURN,                     \
                KWD_EXIT,                       \
                KWD_EXTERN,                     \
                KWD_BREAK,                      \
                KWD_CONTINUE,                   \
                KWD_STRUCT,                     \
                KWD_MODULE,                     \
                KWD_IMPORT,                     \
                KWD_WHERE,                      \
                KWD_EMBED,                      \
}

#define KWD_TYPES {                             \
                KWD_I8,                         \
                KWD_I16,                        \
                KWD_I32,                        \
                KWD_I64,                        \
                KWD_U8,                         \
                KWD_U16,                        \
                KWD_U32,                        \
                KWD_U64,                        \
                KWD_VOID,                       \
}

int kwds_iskw(const char *s);
int kwds_isty(const char *s);

#endif // KWDS_H_INCLUDED
