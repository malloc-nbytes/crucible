module cstring where

-- This module contains a bunch of bindings
-- for functions in string.h.

export extern proc strcmp(s0: u8*, s1: u8*): i32;
export extern proc strlen(s: u8*): i32;
