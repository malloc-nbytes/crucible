module main;

extern proc printf(fmt: u8*, ...): i32;

proc sum(a: i32, b: i32): i32 { return a+b; }

export proc _start(void): ! {
    printf("%d\n", sum(sum(1, 2), 3));

    exit;
}
