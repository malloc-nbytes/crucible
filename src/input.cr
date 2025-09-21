extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    let x: i32 = printf("hi");

    exit;
}
