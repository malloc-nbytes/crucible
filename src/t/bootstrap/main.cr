module main;

import test.basic;
import test.addition;
import test.subtraction;

extern proc printf(fmt: u8*, ...): i32;

proc summary(p: i32, f: i32): void {
    printf("===== Summary =====\n");
    printf("PASSED: %d\n", p);
    printf("FAILED: %d\n", f);
}
proc ok(void): void { printf("ok\n"); }
proc bad(void): void { printf("FAIL\n"); }
proc badi32(got: i32, exp: i32): void {
    printf("FAIL [expected=%d, got=%d]\n", exp, got);
}

export proc _start(void): ! {
    let p: i32 = 0;
    let f: i32 = 0;

    { -- BASIC
        let resi32: i32 = 0;
        if ((resi32 = basic::basic_donothing_r0()) == 0) {
            ok();
            p = p+1;
        } else {
            badi32(resi32, resi32);
            f = f+1;
        }

        if ((resi32 = basic::basic_donothing_r10()) == 10) {
            ok();
            p = p+1;
        } else {
            badi32(resi32, 10);
            f = f+1;
        }
    }

    { -- MATH
        let resi32: i32 = 0;
        if ((resi32 = addition::addition_basic1_r3()) == 3) {
            ok();
            p = p+1;
        } else {
            badi32(resi32, 3);
            f = f+1;
        }

        if ((resi32 = addition::addition_basic2_r5()) == 5) {
            ok();
            p = p+1;
        } else {
            badi32(resi32, 5);
            f = f+1;
        }

        if((resi32 = subtraction::subtraction_basic1_r2()) == 2) {
            ok();
            p = p+1;
        } else {
            badi32(resi32, 2);
            f = f+1;
        }
    }

    summary(p, f);

    exit;
}
