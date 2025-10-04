-- This module tests extremely basic actions.

module basic;

import helpers.log;

-- Purpose: Test doing nothing except returning
--          a value other than 0.
export proc donothing_r10(void): i32 {
    log::id("donothing_r10");
    return 10;
}

-- Purpose: Do nothing except return a value.
export proc donothing_r0(void): i32 {
    log::id("donothing_r0");
    return 0;
}

-- Purpose: To test having a procedure just return.
export proc donothing2_noreturn(void): void {
    log::id("donothing2_noreturn");
    return;
}

-- Purpose: To test having a procedure exist.
export proc donothing1_noreturn(void): void {
    log::id("donothing1_noreturn");
}
