module main;

import "other.cr";

export proc _start(void): ! {
    other::sum(1, 2);

    exit 0;
}
