module log;

extern proc printf(f: u8*, ...): i32;

export proc log(void): void {
    printf("in log()\n");
}
