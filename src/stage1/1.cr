extern proc printf(fmt: u8*, ...): void;
extern proc putchar(c: i32): void;

proc f(a: i32, b: i32): void
{
        printf("a: %d, b: %d", a, b);
        putchar(10);
}

export proc main(): i32
{
        let ar: i32[3] = {1, 2, 3};

        let u: u8 = (u8)ar[0];

        let u: u8 = (u8)1;

        let p: u8* = (u8*)ar;

        return 0;
}
