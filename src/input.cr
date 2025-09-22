extern proc printf(fmt: u8*, ...): i32;
extern proc malloc(b: i32): i32;

proc f(void): void {}

export proc _start(void): ! {
    let x: i32 = 1;

    if (x < 2) {
        printf("hello world %d", 1+2+3);
    }

    exit;
}
