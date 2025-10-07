module test where

export proc _start(void): ! {
        let y: i32 = 10;

        for (let x: i32 = 0; x < 10; x = x + 1) {
                y = y - 1;
        }

        exit y;
}
