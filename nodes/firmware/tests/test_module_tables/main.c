#include "unity.h"

#include "tables.h"

void setUp(void){}

void tearDown(void){}

#define GATE_TEST_ID 0x01, 0x02, 0x03, 0x04
#define MATE_TEST_ID 0x05, 0x06, 0x07, 0x08

extern void test_keys(void);
extern void test_writers(void);
extern void test_iterator(void);
extern void test_merge(void);
extern void test_memo(void);

int main(void)
{
    UNITY_BEGIN();
    test_keys();
    test_writers();
    test_iterator();
    test_merge();
    test_memo();
    return UNITY_END();
}
