module main where

enum Test {
        Item1 = 1+2,
        Item2,
        Item3,
}

proc f(Test t): void
{
}

export proc _start(void): !
{
        f(1);
        exit;
}
