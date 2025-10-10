module main where

import std.binds.c.stdio;
import std.io;

-- proc eqplus1_r1(void): i32
-- {
--         let ar1: [i32] = {1,2,3,4,5};
--         let ar2: [i32] = {2,3,4,5,6};

--         for (let i: i64 = 0; i < 5; i += 1) {
--                 if (ar1[i]+1 != ar2[i]) {
--                         return 0;
--                 }
--         }

--         return 1;
-- }

proc summation_r15(void): i32
{
        let ar: [i32] = {1,2,3,4,5};
        let res: i32 = 0;

        for (let i: i64 = 0; i < 5; i += 1) {
                -- res += ar[i];
        }

        return res;
}

-- proc init2_r0(void): i32
-- {
--         let ar: [i32; 5] = {1,2,3,4,5};
--         return 0;
-- }

-- proc init1_r0(void): i32
-- {
--         let ar: [i32] = {1,2,3};
--         return 0;
-- }

export proc _start(void): !
{
        exit;
}
