module log;

extern proc printf(fmt: u8*, ...): i32;

export proc id(test_id: u8*): void {
    printf("[CRUCIBLE T] %s ... ", test_id);
}
