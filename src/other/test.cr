module other;

extern proc printf(fmt: u8*, ...): i32;

export proc sum(a: i32, b: i32): i32 {
    printf("in sum()\n");
    return a+b;
}
