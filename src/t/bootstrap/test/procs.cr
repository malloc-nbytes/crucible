-- This module tests various different
-- procedure actions.

module procs where

import helpers.log;

----------------
proc __sum_with_procs1(void): i32 { return 2; }
proc __sum_with_procs2(void): i32 { return 3; }

export proc sum_with_procs_r5(void): i32
{
        log::id("procs::sum_with_procs_r5");
        return __sum_with_procs1() + __sum_with_procs2();
}
----------------

----------------
proc ____call_multiple_other_procs_r10(void): i32 { return 10; }
proc __call_multiple_other_procs_r10(void): i32 { return ____call_multiple_other_procs_r10(); }

-- Purpose: Test calling multiple procedures and
--          returning that value.
export proc call_multiple_other_procs_r10(void): i32
{
        log::id("procs::call_multiple_other_procs_r10");
        return __call_multiple_other_procs_r10();
}
----------------

----------------
proc __call_other_proc_r10(void): i32 { return 10; }

-- Purpose: Test calling a procedure and returning
--          that value.
export proc call_other_proc_r10(void): i32
{
        log::id("procs::call_other_proc_r10");
        return __call_other_proc_r10();
}
----------------
