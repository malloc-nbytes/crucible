module main;

import other.other1;
import other.other2;

export proc _start(void): ! {
    other1::test1();
    other2::test2();

    exit;
}
