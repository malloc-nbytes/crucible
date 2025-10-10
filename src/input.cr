module main where

import std.binds.c.stdio;
import std.binds.c.string;

import std.mem;

-- proc f(ar: i32*, n: size_t): void
-- {
--         for (let i: size_t = 0; i < n; i += 1) {
--                 cstdio::printf("%d\n", ar[i]);
--         }
-- }

export proc _start(void): !
{
        -- let ar: [i32; 10] = {1,2,3,4,5,6,7,8,9,10};
        -- let p: u8* = (u8*)ar;
        -- cstring::memset((void*)ar, 0, 10*4);
        -- mem::set((u8*)ar, 0, 10*4);
        -- f((i32*)ar, 10);

        let ar: [i32; 3];
        -- let ar: [i32; 3] = {1,2,3};
        cstdio::printf("%d\n", ar[0]);

        exit;
}
