module math where

export proc sum(a: i32, b: i32): i32
{
    return a + b;
}

export proc sum_range(st: i32, en: i32): i32
{
    let s: i32 = 0;

    for (let i: i32 = st; i <= en; i = i+1) {
        s = s + i;
    }

    return s;
}


export proc abs(a: i32): i32
{
        if (a < 0) return a*-1;
        return a;
}
