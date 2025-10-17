module main where

struct Point {
        x: i32,
        y: i32,
}

export proc _start(void): !
{
        let p: Point = Point {
                .x = 1,
                .y = 2,
        };

        let x: i32 = p.x;

        exit;
}
