module test where

export proc _start(void): ! {
        let x: i32 = 10;
        let y: i32 = 10;

        while (x > 0) {
                if (y == 10) x = x - 1;
        }

        exit x;
}
