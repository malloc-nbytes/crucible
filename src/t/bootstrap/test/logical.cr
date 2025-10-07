module logical where

import helpers.log;

--------------------
proc __or_wprocs_r1_helper1(void): i32 { return 1; }
proc __or_wprocs_r1_helper2(void): i32 { return 0; }

export proc or_wprocs2_r1(void): i32
{
        log::id("logical::or_wprocs2_r1");
        return __or_wprocs_r1_helper2() || __or_wprocs_r1_helper1();
}

export proc or_wprocs1_r1(void): i32
{
        log::id("logical::or_wprocs1_r1");
        return __or_wprocs_r1_helper1() || __or_wprocs_r1_helper2();
}
--------------------

export proc and2_r0(void): i32
{
        log::id("logical::and2_r1");
        return 0 && 1;
}

export proc and1_r0(void): i32
{
        log::id("logical::and1_r1");
        return 1 && 0;
}

export proc or2_r1(void): i32
{
        log::id("logical::or2_r1");
        return 0 || 1;
}

export proc or1_r1(void): i32
{
        log::id("logical::or1_r1");
        return 1 || 0;
}
