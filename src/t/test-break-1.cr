module test;

export proc _start(void): ! {
    let x: i32 = 5;

    while (x > 0-10) {
        if (x == 0) break;
        x = x-1;
    }

    exit x;
}
