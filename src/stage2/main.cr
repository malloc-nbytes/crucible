extern proc printf(fmt: u8*, ...): void;
extern proc putchar(c: u8): void;
extern proc load_file(path: u8*): u8*;

export proc main(): i32
{
        let x: i32 = 1;

        let p: i32* = &x;

        *p = 2;

        printf("%d", x);
        putchar((u8)10);

        return 0;
}
