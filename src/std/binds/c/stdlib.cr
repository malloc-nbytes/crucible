module cstdlib where

-- This module contains a bunch of bindings
-- for functions in stdlib.h.

export extern proc malloc(size: size_t): void*;
export extern proc free(ptr: void*): void;
export extern proc realloc(ptr: void*, size: size_t): i32*;
