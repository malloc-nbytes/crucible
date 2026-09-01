extern proc printf(fmt: u8*, ...): void;
extern proc putchar(c: i32): void;

export proc main(): i32
{
        let x: i32 = 0;

        while x < 10 {
                printf("%d", x);
                putchar(10);
                x = x + 1;
        }

        return 0;
}
