module test;

export proc _start(void): ! {
    for (let x: i32 = 0; x < 100; x = x + 1) {}

    exit 0;
}
