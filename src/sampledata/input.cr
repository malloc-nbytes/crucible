module main;

import other;

extern proc printf(fmt: u8*, ...): i32;

proc sum(a: i32, b: i32): i32 {
    return 100;
}

export proc _start(void): ! {
    printf("_start()\n");

    printf("other::sum(): %d\n", other::sum(1, 2));
    printf("this->sum(): %d\n", sum(1, 2));

    exit;
}
