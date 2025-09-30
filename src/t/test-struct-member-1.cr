struct F {
    x: i32,
    s: u8*,
    y: i32,
}

export proc _start(void): ! {
    let f: F = (F) {
        .x = 1,
        .s = "foo",
        .y = 0,
    };

    exit f.y;
}
