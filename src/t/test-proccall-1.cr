proc f(void): i32 {
    return 0;
}

export proc _start(void): ! {
    exit f();
}
