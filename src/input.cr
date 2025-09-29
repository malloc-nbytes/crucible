extern proc printf(fmt: u8*, ...): i32;

struct Point {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {

    let test1: i32 = 99;

    let p: Point = {
        .x = 1,
        .s = "foo",
        .y = 2,
    };

    let test2: i32 = 100;

    printf("test1: %d\n", test1);
    printf("test2: %d\n", test2);

    exit 0;
}
