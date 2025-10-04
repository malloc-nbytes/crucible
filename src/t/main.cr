import "test-basic-addition-1.cr";

export proc _start(void): ! {
    if (test_basic_addition_1::test_basic_addition_1() != 15) {
        printf("FAIL: exp=15, got=%d\n", );
    }
}
