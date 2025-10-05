module c;

-- This module contains a bunch of wrappers
-- for functions in glibc.

export extern proc printf(fmt: u8*, ...): i32;
export extern proc strcmp(s0: u8*, s1: u8*): i32;
export extern proc malloc(bytes: i32): void*;
export extern proc free(ptr: void*): void;
