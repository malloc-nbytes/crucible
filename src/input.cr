extern proc printf(fmt: u8*, ...): i32;
extern proc malloc(b: i32): i32;

export proc _start(void): ! {
    malloc(100);

    exit;
}
