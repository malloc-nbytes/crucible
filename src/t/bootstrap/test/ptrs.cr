module ptrs where

import helpers.log;
import helpers.assert;

export proc
ptr_arith3_t_12345_r2(p: i32*, n: i64): i32
{
        log::id("ptrs::ptr_arith3_t_12345_r2");
        return p[-1];
}

export proc
ptr_arith2_t_12345_r2(p: i32*, n: i64): i32
{
        log::id("ptrs::ptr_arith2_t_12345_r2");
        return *(p-1);
}

export proc
ptr_arith1_t_12345_r2(p: i32*, n: i64): i32
{
        log::id("ptrs::ptr_arith1_t_12345_r2");
        assert::i32eq(*p, 3);
        let p2: i32* = p-1;
        return *p2;
}

export proc
sum_array2_t12345_t5_r15(p: i32*, n: i64): i32
{
        log::id("ptrs::sum2_array_t12345_t5_r15");

        let sum: i32 = 0;
        for (let i: i64 = 0; i < n; i += 1) {
                sum += *(p+i);
        }
        return sum;
}

export proc
sum_array1_t12345_t5_r15(p: i32*, n: i64): i32
{
        log::id("ptrs::sum1_array_t12345_t5_r15");

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
