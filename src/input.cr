module main where

import std.binds.c.{stdlib, stdio};

proc f(a: i32, b: i32, c: i32, d: i32, s: proc(void):void): void {}
proc a(void): void {}
export proc _start(void): !
{
        let _a: proc(void): void = a;
        f(1,2,3,4,_a);
        exit;
}
