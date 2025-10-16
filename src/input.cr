module main where

struct Point {
        x: i32,
        s: u8*,
        y: i32,
}

export proc _start(void): !
{
        let p: Point = Point {
                .x = 1,
                .s = "foo",
                .y = 2,
        };

        let copy: Point = p;

        exit;
}
