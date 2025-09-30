extern proc strcmp(s0: u8*, s1: u8*): i32;

export proc _start(void): ! {
    let s0: u8* = "hello";
    let s1: u8* = "hello";
    let s2: u8* = "foo";

    if (strcmp(s0, s2) == 0) {
        exit 1;
    }

    exit strcmp(s0, s1);
}
