module cstdlib where

-- This module contains a bunch of bindings
-- for functions in stdlib.h.

export extern proc malloc(size: size_t): void*;
export extern proc free(ptr: void*): void;
export extern proc realloc(ptr: void*, size: size_t): i32*;
       -- void qsort(void base[.size * .n], size_t n, size_t size,
       --            typeof(int (const void [.size], const void [.size]))
       --                *compar);
export extern proc qsort(base: void*, n: size_t, size: size_t, compar: proc(void*, void*): i32): void;
