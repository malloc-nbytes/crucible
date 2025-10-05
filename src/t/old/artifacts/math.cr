module math where

proc f1(void): void {}
proc f2(x: i32): i32 {
    return x+1-1;
}

export proc do_nothing_but_call_other_procs(a: i32): i32 {
    f1();
    return f2(a);
}

export proc sum_complex(a: i32, b: i32): i32 {
    a = a - 2;
    b = b - 2;
    a = a + 2;
    b = b + 2;
    return a+b;
}

export proc sum(a: i32, b: i32): i32 {
    return a+b;
}
