extern proc printf(fmt: u8*, ...): i32;

struct Point {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {

    let test1: i32 = 99;
    let test2: i32 = 0;

    let p: Point = {
        .x = 1,
        .s = "foo",
        .y = 2,
    };

    printf("in input: %d\n", test1);

    exit 0;
}
