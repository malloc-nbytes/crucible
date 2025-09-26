extern proc printf(fmt: u8*, ...): i32;

struct Point {
    x: i32,
    s: u8*,
}

export proc _start(void): ! {
    exit 0;
}
