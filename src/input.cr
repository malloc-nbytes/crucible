extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    let s: i32 = sum("hello");

    exit 0;
}
