module basic;

import helpers.log;

export proc basic_donothing_r10(void): i32 {
    log::id("basic_donothing_r10");
    return 10;
}

export proc basic_donothing_r0(void): i32 {
    log::id("basic_donothing_r0");
    return 0;
}
