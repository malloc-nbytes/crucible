extern proc printf(fmt: u8*, ...): i32;

struct F {
    o: i32,
}

struct Point {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {
    let p: Point = {
        .x = 1,
        .s = "foo",
        .y = 3,
    };

    printf("p.s = %s\n", p.s);

    exit 0;
}
