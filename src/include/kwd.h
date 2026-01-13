#ifndef KWD_H_INCLUDED
#define KWD_H_INCLUDED

#define KWD_IF "if"
#define KWD_ELSE "else"
#define KWD_LET "let"
#define KWD_WHILE "while"
#define KWD_FOR "for"
#define KWD_PROC "proc"

#define KWD_CPL { \
                KWD_IF,                         \
                KWD_ELSE,                       \
                KWD_LET,                        \
                KWD_WHILE,                      \
                KWD_FOR,                        \
                KWD_PROC,                       \
        }

int iskwd(const char *s);

#endif // KWD_H_INCLUDED
