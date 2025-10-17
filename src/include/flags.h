#ifndef FLAGS_H_INCLUDED
#define FLAGS_H_INCLUDED

typedef enum {
        FLAG_TYPE_ASM     = 1 << 0,
        FLAG_TYPE_NOSTD   = 1 << 1,
        FLAG_TYPE_VERBOSE = 1 << 2,
} flag_type;

#define FLAG_1HY_HELP 'h'
#define FLAG_2HY_HELP "help"

#define FLAG_1HY_OUTPUT 'o'
#define FLAG_2HY_OUTPUT "output"

#define FLAG_2HY_ASM "asm"

#define FLAG_1HY_SEARCHPATH 'I'

#define FLAG_2HY_NOSTD "nostd"

#define FLAG_1HY_LIBPATH 'L'
#define FLAG_1HY_LIB 'l'
#define FLAG_2HY_LIBPATH "library-path"
#define FLAG_2HY_LIB "lib"

#define FLAG_1HY_VERBOSE 'v'
#define FLAG_2HY_VERBOSE "verbose"

#endif // FLAGS_H_INCLUDED
