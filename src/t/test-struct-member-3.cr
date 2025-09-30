extern proc strcmp(s0: u8*, s1: u8*): i32;

struct F {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {
    let f: F = {
        .x = 5,
        .s = "foo",
        .y = 0,
    };

    exit strcmp(f.s, "foo");
}
