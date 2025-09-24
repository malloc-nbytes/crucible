extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    printf("foo\tbar\n");

    exit;
}
