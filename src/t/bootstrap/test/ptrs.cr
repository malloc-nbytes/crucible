module ptrs where

import helpers.log;

export proc
sum_array_t12345_t5_r15(p: i32*, n: i64): i32
{
        log::id("ptrs::sum_array_t12345_t5_r15");

        let sum: i32 = 0;
        for (let i: i64 = 0; i < n; i += 1) {
                sum += p[i];
        }
        return sum;
}

export proc take_ptr2_t1_r1(p: i32*): i32
{
        log::id("ptrs::take_ptr2_t1_r1");
        return p[0] == 1;
}

export proc take_ptr1_r1(p: i32*): i32
{
        log::id("ptrs::take_ptr1_r1");
        return 1;
}
