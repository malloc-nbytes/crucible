extern proc printf(fmt: u8*, ...): i32;

struct Point {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {
    let _: i32 = 0;

    let p: Point = {
        .x = 1,
        .s = "foo",
        .y = 2,
    };

    exit 0;
}
