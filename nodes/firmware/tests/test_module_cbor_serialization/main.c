#include "unity.h"

void setUp(void){}

void tearDown(void){}

extern void test_serialize(void);
extern void test_deserialize(void);

int main(void)
{
    UNITY_BEGIN();
    test_serialize();
    test_deserialize();
    return UNITY_END();
}
