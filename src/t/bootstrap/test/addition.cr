-- This module tests various addition actions.

module addition;

import helpers.log;

-- Purpose: Test long addition with many different
--          variables.
export proc long2_r120(void): i32
{
    log::id("addition::long2_r120");
    let a: i32 = 1+2+3+4+5;
    let b: i32 = 6+7+8+9+10;
    let c: i32 = 11+12+13;
    let d: i32 = 14+15;
    return a+b+c+d;
}

-- Purpose: Test long addition.
export proc long1_r120(void): i32
{
    log::id("addition::long1_r120");
    return 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15;
}

-- Purpose: Test basic addition with a variable.
export proc basic2_r5(void): i32
{
    log::id("addition::basic2_r5");
    let s: i32 = 1+2;
    return s + 2;
}

-- Purpose: Test basic addition.
export proc basic1_r3(void): i32
{
    log::id("addition::basic1_r3");
    return 1 + 2;
}
