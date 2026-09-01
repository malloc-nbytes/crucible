extern proc malloc(b: i32): u8*;

proc sum(a: i32): i32
{
        let ptr: u8* = malloc(43);

        return a;
}

