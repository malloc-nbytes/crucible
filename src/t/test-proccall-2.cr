proc f(a: i32): i32 {
    return a;
}

export proc _start(void): ! {
    exit f(0);
}
