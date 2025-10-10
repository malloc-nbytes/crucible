module cstring where

-- This module contains a bunch of bindings
-- for functions in string.h.

export extern proc strcmp(s0: u8*, s1: u8*): i32;
export extern proc strlen(s: u8*): i32;
export extern proc strdup(s: u8*): u8*;
export extern proc strcat(dst: u8*, src: u8*): u8*;
export extern proc memset(s: void*, c: i32, n: size_t): void*;
