extern proc printf(fmt: u8*, ...): i32;

struct F { o: i32 }

struct Point {
    x: i32,
    f: F,
}

export proc _start(void): ! {
    let p: Point = (Point) {
        .x = 9,
        .f = (F) { .o = 9 }
    };

    exit 0;
}
