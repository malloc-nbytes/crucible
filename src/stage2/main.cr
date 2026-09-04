extern proc printf(fmt: u8*, ...): void;
extern proc malloc(bytes: i32): void*;
extern proc putchar(c: u8): void;
extern proc load_file(path: u8*): u8*;

struct Token {
        lx: u8*,
        k: u32,
}

struct Lexer {
        t: i32,
}

proc lex_file(path: u8*, src: u8*): void
{
        let lexer: void* = malloc(4);
}

export proc main(): i32
{
        let path: u8* = "1.in";
        let src: u8* = load_file(path);

        lex_file(path, src);

        return 0;
}
