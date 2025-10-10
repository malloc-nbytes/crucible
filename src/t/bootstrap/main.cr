-- This is the main test driver for bootstrapped tests.
-- Compilation:
--   ../../cruc ./main.cr -o TEST.bin --asm --nostd -I ../../
-- This file will change in the future as the language
-- becomes more complex and adds more useful features
-- so we don't have to use a bunch of if-else statements
-- and repeated assignments of `p` and `f`.

-- Note: Maybe use macros once implemented.

module main where

import std.binds.c.stdio;

import helpers.assert;

import test.basic;
import test.addition;
import test.subtraction;
import test.procs;
import test.logical;
import test.arrays;
import test.ptrs;
import test.chars;

proc ok(void): void { cstdio::printf("ok\n"); }

proc bad(got: i32, exp: i32): void
{
        cstdio::printf("FAIL [expected=%d, got=%d]\n", exp, got);
}

proc summary(p: size_t, f: size_t): void
{
        cstdio::printf("===== Summary =====\n");
        cstdio::printf("PASSED: %d\n", p);
        cstdio::printf("FAILED: %d\n", f);
}

export proc _start(void): !
{
        let p: size_t = 0;
        let f: size_t = 0;

        { -- BASIC
                let resi32: i32 = 0;

                basic::donothing1_noreturn(); ok();
                basic::donothing2_noreturn(); ok();

                if ((resi32 = basic::donothing_r0()) == 0) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, resi32);
                        f = f+1;
                }

                if ((resi32 = basic::donothing_r10()) == 10) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 10);
                        f = f+1;
                }
        }

        { -- MATH
                let resi32: i32 = 0;

                if ((resi32 = addition::basic1_r3()) == 3) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 3);
                        f = f+1;
                }

                if ((resi32 = addition::basic2_r5()) == 5) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 5);
                        f = f+1;
                }

                if ((resi32 = addition::long1_r120()) == 120) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 120);
                        f = f+1;
                }

                if ((resi32 = addition::long2_r120()) == 120) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 120);
                        f = f+1;
                }

                if((resi32 = subtraction::basic1_r2()) == 2) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 2);
                        f = f+1;
                }
        }

        { -- PROCS
                let resi32: i32 = 0;

                if ((resi32 = procs::call_other_proc_r10()) == 10) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 10);
                        f = f+1;
                }

                if ((resi32 = procs::call_multiple_other_procs_r10()) == 10) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 10);
                        f = f+1;
                }

                if ((resi32 = procs::sum_with_procs_r5()) == 5) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 5);
                        f = f+1;
                }

        }

        { -- LOGICAL
                let resi32: i32 = 0;

                if ((resi32 = logical::or1_r1()) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

                if ((resi32 = logical::or2_r1()) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

                if ((resi32 = logical::and1_r0()) == 0) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 0);
                        f = f+1;
                }

                if ((resi32 = logical::and2_r0()) == 0) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 0);
                        f = f+1;
                }

                if ((resi32 = logical::or_wprocs1_r1()) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

                if ((resi32 = logical::or_wprocs2_r1()) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

        }

        { -- ARRAYS
                let resi32: i32 = 0;

                if ((resi32 = arrays::init1_r0()) == 0) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 0);
                        f = f+1;
                }

                if ((resi32 = arrays::init2_r0()) == 0) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 0);
                        f = f+1;
                }

                if ((resi32 = arrays::summation_r15()) == 15) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 15);
                        f = f+1;
                }

                if ((resi32 = arrays::eqplus1_r1()) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }
        }

        { -- PTRS
                let resi32: i32 = 0;
                let t1: i32 = 1;
                let t2: [i32] = {1,2,3,4,5};

                if ((resi32 = ptrs::take_ptr1_r1(&t1)) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

                if ((resi32 = ptrs::take_ptr2_t1_r1(&t1)) == 1) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 1);
                        f = f+1;
                }

                if ((resi32 = ptrs::sum_array1_t12345_t5_r15(&t2[0], 5)) == 15) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 15);
                        f = f+1;
                }

                if ((resi32 = ptrs::sum_array2_t12345_t5_r15(&t2[0], 5)) == 15) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 15);
                        f = f+1;
                }

                if ((resi32 = ptrs::ptr_arith1_t_12345_r2(&t2[2], 5)) == 2) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 2);
                        f = f+1;
                }

                if ((resi32 = ptrs::ptr_arith2_t_12345_r2(&t2[2], 5)) == 2) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 2);
                        f = f+1;
                }

                if ((resi32 = ptrs::ptr_arith3_t_12345_r2(&t2[2], 5)) == 2) {
                        ok();
                        p = p+1;
                } else {
                        bad(resi32, 2);
                        f = f+1;
                }

        }

        { -- CHARS
                let resu8: u8 = 0;
                let resi8: i8 = 0;

                if ((resu8 = chars::basic1_u8_r97()) == 'a') {
                        ok();
                        p = p+1;
                } else {
                        bad((i32)resu8, 97);
                        f = f+1;
                }

                if ((resu8 = chars::basic2_u8_r97()) == 'a') {
                        ok();
                        p = p+1;
                } else {
                        bad((i32)resu8, 97);
                        f = f+1;
                }

                if ((resi8 = chars::basic1_i8_r97()) == (i8)'a') {
                        ok();
                        p = p+1;
                } else {
                        bad((i32)resi8, 97);
                        f = f+1;
                }

                if ((resi8 = chars::basic2_i8_r97()) == (i8)'a') {
                        ok();
                        p = p+1;
                } else {
                        bad((i32)resi8, 97);
                        f = f+1;
                }

                if ((resu8 = chars::u8_from_array_r97()) == 'a') {
                        ok();
                        p = p+1;
                } else {
                        bad((i32)resu8, 97);
                        f = f+1;
                }

        }

        summary(p, f);

        exit;
}
