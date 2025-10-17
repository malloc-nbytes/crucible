module chars where

import helpers.log;

export proc u8_from_array_r97(void): u8
{
        log::id("chars::u8_from_array_r97");
        let arr: [u8] = ['b', 'a', 'c'];
        return arr[1];
}

export proc basic2_i8_r97(void): i8
{
        log::id("chars::basic2_i8_r97");
        return (i8)'a';
}

export proc basic1_i8_r97(void): i8
{
        log::id("chars::basic1_i8_r97");
        let c: i8 = (i8)'a';
        return c;
}

export proc basic2_u8_r97(void): u8
{
        log::id("chars::basic2_u8_r97");
        return 'a';
}

export proc basic1_u8_r97(void): u8
{
        log::id("chars::basic1_u8_r97");
        let c: u8 = 'a';
        return c;
}
