#ifndef FLAGS_H_INCLUDED
#define FLAGS_H_INCLUDED

typedef enum {
        FLAG_TYPE_ASM = 1 << 0,
} flag_type;

#define FLAG_1HY_HELP 'h'
#define FLAG_2HY_HELP "help"

#define FLAG_1HY_OUTPUT 'o'
#define FLAG_2HY_OUTPUT "output"

#define FLAG_2HY_ASM "asm"

#endif // FLAGS_H_INCLUDED
