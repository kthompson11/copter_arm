#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <crc32.c>

#ifdef COMMENTED_OUT
static void test_crc32_init(void **state)
{
    crc32_init();
}
#endif

static void test_no_input(void **state)
{
    uint8_t *sequence = 0;
    uint32_t res = crc32(sequence, 0);

    assert_true(res == 0);
}

static void test_1_byte(void **state)
{
    uint8_t val = 1;
    uint32_t res = crc32(&val, 1);
    assert_true(res == 0xa505df1b);
}

static void test_2_bytes(void **state)
{
    uint8_t data[2] = {0x01, 0x12};
    uint32_t res = crc32(data, 2);
    assert_true(res == 0xab7b52f6);
}

static void test_random_sequence(void **state)
{
    uint8_t sequence[] = {18, 86, 120, 102, 119, 91, 51, 112, 40, 183, 139, 192, 
                          78, 227, 214, 51, 152, 24, 169, 135, 36, 201, 192, 72, 
                          150, 117, 40, 238, 1, 71, 146, 61, 159, 225, 132, 108, 
                          85, 253, 80, 59, 100, 244, 96, 240, 6, 141, 185, 99, 
                          41, 187, 139, 123, 113, 60, 167, 48, 15, 215, 244, 110, 
                          212, 31, 7, 88, 51, 243, 251, 212, 227, 73, 168, 69, 
                          167, 141, 133, 184, 226, 174, 139, 248, 31, 85, 207, 
                          199, 57, 215, 234, 202, 125, 193, 95, 136, 139, 142, 
                          94, 210, 128, 159, 124, 220};
    const int SEQ_LEN = sizeof(sequence) / sizeof(sequence[0]);

    uint32_t res = crc32(sequence, SEQ_LEN);
    assert_true(res == 0xc18de39d);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_input),
        cmocka_unit_test(test_1_byte),
        cmocka_unit_test(test_2_bytes),
        cmocka_unit_test(test_random_sequence),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
