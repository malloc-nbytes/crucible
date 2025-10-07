module test where

import artifacts.math;

export proc _start(void): ! {
        exit math::sum(0-5, 5);
}
