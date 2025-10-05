module test where

export proc _start(void): ! {
    let k: i32 = 10*10;

    for (let x: i32 = 0; x < 10; x = x + 1) {
        for (let y: i32 = 0; y < 10; y = y + 1) {
            k = k - 1;
        }
    }

    exit k;
}
