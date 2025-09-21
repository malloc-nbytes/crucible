export proc _start(void): ! {
    let x: i32 = 0;
    let y: i32 = x;
    let z: i32 = y;
    let a: i32 = z;
    let b: i32 = a;
    let c: i32 = b;
    let d: i32 = c;
    let e: i32 = d;
    let f: i32 = e;
    let g: i32 = f;
    let h: i32 = g;

    exit h;
}
