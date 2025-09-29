extern proc printf(fmt: u8*, ...): i32;

struct Point {
    x: i32,
    y: i32,
}

export proc _start(void): ! {
    let p: Point = {
        .x = 1,
        .y = 2,
    };

    exit 0;
}
