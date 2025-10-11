module cstdio where

-- This module contains a bunch of bindings
-- for functions in stdio.h.

export extern proc printf(fmt: u8*, ...): i32;
export extern proc puts(s: u8*): i32;
