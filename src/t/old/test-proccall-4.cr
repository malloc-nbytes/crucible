module test where

proc _f(a: i32): i32 {
        return a-3;
}

proc f(a: i32, b: i32): i32 {
        return _f(3)+b-b;
}

export proc _start(void): ! {
        exit f(0-5, 5);
}
