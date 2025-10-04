module test;

export proc _start(void): ! {
    let x: i32 = 2;

    if (x == 1) {
        exit 1;
    } else if (x == 2) {
        exit 0;
    } else {
        exit 1;
    }

    exit 1;
}
