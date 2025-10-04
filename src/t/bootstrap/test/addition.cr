module addition;

import helpers.log;

export proc addition_basic2_r5(void): i32 {
    log::id("addition_basic2");
    let s: i32 = 1+2;
    return s + 2;
}

export proc addition_basic1_r3(void): i32 {
    log::id("addition_basic1_r3");
    return 1 + 2;
}
