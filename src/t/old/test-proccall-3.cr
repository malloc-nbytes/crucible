module test where

proc f(a: i32, b: i32): i32 {
    return a+b;
}

export proc _start(void): ! {
    exit f(0-5, 5);
}
