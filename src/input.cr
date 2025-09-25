extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    let x: i32 = 0;
    while (x > 10) {
        if (x == 5) continue;
        printf("%d\n", x);
        x = x+1;
    }

    exit 0;
}
