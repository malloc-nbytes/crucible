module cstdlib where

-- This module contains a bunch of bindings
-- for functions in stdlib.h.

export extern proc malloc(bytes: i32): void*;
export extern proc free(ptr: void*): void;
