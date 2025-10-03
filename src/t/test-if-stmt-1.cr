module test;

export proc _start(void): ! {
    let x: i32 = 9;

    if (x == 9) {
        exit 0;
    }

    exit 1;
}
