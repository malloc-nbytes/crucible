struct F {
    y: i32,
    x: i32,
    s: u8*,
}

export proc _start(void): ! {
    let f: F = {
        .y = 0,
        .x = 5,
        .s = "foo",
    };

    exit f.x - 5;
}
