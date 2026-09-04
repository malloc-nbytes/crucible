extern proc printf(fmt: u8*, ...): void;
extern proc putchar(c: i32): void;

struct Point {
        x: i32,
        y: i32,
        name: u8*,
}

export proc main(): i32
{
        let p: Point = Point {
                .x = 1,
                .y = 2,
                .name = "my point",
        };

        return 0;
}
