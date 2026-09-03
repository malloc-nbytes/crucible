extern proc printf(fmt: u8*, ...): void;
extern proc putchar(c: i32): void;

export proc main(): i32
{
        let ar: i32[3] = {1, 2, 3};

        let i: i32 = 0;

        while i < 3 {
                printf("%d", ar[i]);
                putchar(10);
                i += 1;
        }

        return 0;
}
