module main;

import "other/test.cr";

export proc _start(void): ! {
    let x: i32 = other::sum(1, 2);

    exit 0;
}
