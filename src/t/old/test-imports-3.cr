module test;

import artifacts.strs;
import artifacts.math;

extern proc strcmp(s0: u8*, s1: u8*): i32;

export proc _start(void): ! {
    let s: i32 = strcmp("hello world", strs::get_hello_world());
    exit math::sum(0-5, 5) + s;
}
