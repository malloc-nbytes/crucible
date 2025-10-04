module main;

import other.test;

extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    printf("in main()\n");
    let x: i32 = other::sum(1, 2);
    printf("x = %d\n", x);

    exit;
}
