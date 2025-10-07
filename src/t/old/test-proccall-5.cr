module test where

proc f(a: i64): i64 {
        return a;
}

export proc _start(void): ! {
        let ret: i64 = f(0);

        exit ret;
}
