module test;

export proc _start(void): ! {
    let x: i32 = 1;

    if (x >= 2) {
        exit 1;
    } else {
        exit 0;
    }

    exit 1;
}
