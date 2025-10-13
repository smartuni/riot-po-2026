#include "embUnit.h"

#include "tables.h"

#define ENABLE_DEBUG 0
#include "debug.h"


#define TEST_DATA_NUMOF ARRAY_SIZE(test_data)

static void test_tables_init(void)
{
    init_tables();
}

Test *tests_tables_init(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_tables_init),
    };

    EMB_UNIT_TESTCALLER(tables_tests, NULL, NULL, fixtures);

    return (Test *)&tables_tests;
}

int main(void)
{
    TESTS_START();
    TESTS_RUN(tests_tables_init());
    return TESTS_END();
}
