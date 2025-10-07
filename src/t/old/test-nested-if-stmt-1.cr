module test where

export proc _start(void): ! {
        if (1 == 1) {
                if (2 == 2) {
                        exit 0;
                } else {
                        exit 1;
                }
                exit 1;
        }
        exit 1;
}
