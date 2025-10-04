module test;

import t.artifacts.strs;
import t.artifacts.math;

extern proc strcmp(s0: u8*, s1: u8*): i32;
extern proc printf(fmt: u8*, ...): i32;

export proc _start(void): ! {
    -- let s: i32 = strcmp("hello world", strs::get_hello_world());
    let s: i32 = 5;
    printf("%d\n", math::sum(1, 2) + s);
    exit;
}
