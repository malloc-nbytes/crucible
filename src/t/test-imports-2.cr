module test;

import artifacts.math;

export proc _start(void): ! {
    let s: i32 = math::do_nothing_but_call_other_procs(0-5+5);

    exit s;
}
