export proc _start(void): ! {
    let x: i32 = 1;
    let y: i32 = x+1;

    if (x == 1) {
        if (y == 2) {
            exit 0;
        } else {
            exit 1;
        }
        exit 1;
    }
    exit 1;
}
