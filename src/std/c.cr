module c;

-- This module contains a bunch of wrappers
-- for functions in glibc.

export extern proc printf(fmt: u8*, ...): i32;
