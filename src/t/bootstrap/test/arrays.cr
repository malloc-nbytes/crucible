module arrays where

import helpers.log;

export proc eqplus1_r1(void): i32
{
        log::id("arrays::eqplus1_r1");

        let ar1: [i32] = {1,2,3,4,5};
        let ar2: [i32] = {2,3,4,5,6};

        for (let i: size_t = 0; i < 5; i += 1) {
                if (ar1[i]+1 != ar2[i]) {
                        return 0;
                }
        }

        return 1;
}

export proc summation_r15(void): i32
{
        log::id("arrays::summation_r15");

        let ar: [i32] = {1,2,3,4,5};
        let res: i32 = 0;

        for (let i: size_t = 0; i < 5; i += 1) {
                res += ar[i];
        }

        return res;
}

export proc init2_r0(void): i32
{
        log::id("arrays::init2_r0");
        let ar: [i32; 5] = {1,2,3,4,5};
        return 0;
}

export proc init1_r0(void): i32
{
        log::id("arrays::init1_r0");
        let ar: [i32] = {1,2,3};
        return 0;
}
