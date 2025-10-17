#include "unity.h"

#include "tables.h"

#define ENABLE_DEBUG 0
#include "debug.h"

void setUp(void){}
void tearDown(void){}

static void test_tables_init(void)
{
    TEST_ASSERT_EQUAL_INT(init_tables(), TABLE_SUCCESS);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_tables_init);
    return UNITY_END();
}
