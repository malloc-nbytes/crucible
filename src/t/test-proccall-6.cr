proc f(a: i32): i32 {
    let x: i32 = 3%4;

    if a == 3 {
        return 1;
    } else if a == 2 {
        return 1;
    } else {
        return 0;
    }
}

export proc _start(void): ! {
    exit f(1);
}
