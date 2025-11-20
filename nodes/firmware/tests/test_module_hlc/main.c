
#include "unity.h"

#include "hlc_ztimer.h"
#include "hybrid_logical_clock.h"

void setUp(void){}

void tearDown(void){}

int _hlc_failing_get_time(void *arg, hlc_physical_t *out)
{
    (void) arg;

    TEST_ASSERT_NOT_NULL(out);
    return HLC_ERROR_GET_TIME;
}

int _hlc_successful_once_get_time(void *arg, hlc_physical_t *out)
{
    (void) arg;

    static int call_count = 0;
    TEST_ASSERT_NOT_NULL(out);

    call_count++;
    if (call_count == 1) {
        *out = 1000;
        return 0;
    }
    return HLC_ERROR_GET_TIME;
}

static void test_hlc_init(void)
{
    hlc_ctx_t ctx;
    hlc_ztimer_t ztimer_ctx;

    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx, hlc_ztimer_get_time, &ztimer_ctx));
}

static void test_hlc_ztimer(void)
{
    hlc_physical_t physical_1, physical_2;
    hlc_ztimer_t ztimer_ctx;

    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));

    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_get_time(&ztimer_ctx, &physical_1));
    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_get_time(&ztimer_ctx, &physical_2));

    while (&physical_1 >= &physical_2) {
        TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_get_time(&ztimer_ctx, &physical_2));
    }
}

static void test_hlc_get_current_timestamp(void)
{
    hlc_ztimer_t ztimer_ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));

    hlc_ctx_t ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx, hlc_ztimer_get_time, &ztimer_ctx));

    hlc_timestamp_t ts_0, ts_1;
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx, &ts_0));
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx, &ts_1));

    // ts_1 should be greater than ts_0 due to logical increment
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&ts_1, &ts_0));
    TEST_ASSERT_EQUAL_INT(-1, hlc_compare(&ts_0, &ts_1));

    TEST_ASSERT_EQUAL_INT(0, hlc_compare(&ts_0, &ts_0));
    TEST_ASSERT_EQUAL_INT(0, hlc_compare(&ts_1, &ts_1));
}

static void test_hlc_update_current_timestamp_with_later(void)
{
    hlc_ztimer_t ztimer_ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));

    hlc_ctx_t ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx, hlc_ztimer_get_time, &ztimer_ctx));

    hlc_timestamp_t ts_0, ts_1;
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx, &ts_0));

    // simulate receiving a remote timestamp greater than local
    ts_1.physical = ts_0.physical + 10;
    ts_1.logical  = 5;

    hlc_timestamp_t updated_ts;
    TEST_ASSERT_EQUAL_INT(0, hlc_update_with_remote_timestamp(&ctx, &ts_1, &updated_ts));

    // updated_ts should now be later than the remote timestamp
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&updated_ts, &ts_1));
}


static void test_hlc_update_current_timestamp_with_earlier(void)
{
    hlc_ztimer_t ztimer_ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));

    hlc_ctx_t ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx, hlc_ztimer_get_time, &ztimer_ctx));

    hlc_timestamp_t ts_0, ts_1;
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx, &ts_0));

    // simulate receiving a remote timestamp greater than local
    ts_1.physical = ts_0.physical + 10;
    ts_1.logical  = 0;
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&ts_1, &ts_0));

    hlc_timestamp_t updated_ts;
    TEST_ASSERT_EQUAL_INT(0, hlc_update_with_remote_timestamp(&ctx, &ts_1, &updated_ts));

    // updated_ts should still be later than the remote timestamp
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&updated_ts, &ts_1));

    // updated_ts should still be later than the original local timestamp ts_0
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&updated_ts, &ts_0));
}

static void test_hlc_failing_time(void)
{
    hlc_ctx_t ctx;
    TEST_ASSERT_EQUAL_INT(HLC_ERROR_GET_TIME, hlc_init(&ctx, _hlc_failing_get_time, NULL));

    hlc_timestamp_t ts;
    TEST_ASSERT_EQUAL_INT(HLC_ERROR_GET_TIME, hlc_get_current_timestamp(&ctx, &ts));

    hlc_timestamp_t remote_ts = { .physical = 1000, .logical = 0 };
    TEST_ASSERT_EQUAL_INT(HLC_ERROR_GET_TIME, hlc_update_with_remote_timestamp(&ctx, &remote_ts, &ts));
}

static void test_hlc_sometimes_failing_time(void)
{
    hlc_ctx_t ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx, _hlc_successful_once_get_time, NULL));

    hlc_timestamp_t ts;
    TEST_ASSERT_EQUAL_INT(HLC_ERROR_GET_TIME, hlc_get_current_timestamp(&ctx, &ts));

    hlc_timestamp_t remote_ts = { .physical = 1000, .logical = 0 };
    TEST_ASSERT_EQUAL_INT(HLC_ERROR_GET_TIME, hlc_update_with_remote_timestamp(&ctx, &remote_ts, &ts));
}

static void test_complex_time_series(void)
{
    hlc_ztimer_t ztimer_ctx;
    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(&ztimer_ctx, 0));

    // events on Node A
    hlc_timestamp_t a1, a2;
    hlc_ctx_t ctx_a;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx_a, hlc_ztimer_get_time, &ztimer_ctx));
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx_a, &a1));
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx_a, &a2));

    // events on Node B
    hlc_timestamp_t b1, b2;
    hlc_ctx_t ctx_b;
    TEST_ASSERT_EQUAL_INT(0, hlc_init(&ctx_b, hlc_ztimer_get_time, &ztimer_ctx));
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx_b, &b1));
    TEST_ASSERT_EQUAL_INT(0, hlc_get_current_timestamp(&ctx_b, &b2));

    // Node B receives a1
    hlc_timestamp_t after_a1;
    // Instances are independent, no guarrantee about this comparison.
    TEST_ASSERT_EQUAL_INT(0, hlc_update_with_remote_timestamp(&ctx_b, &a1, &after_a1));
    TEST_ASSERT_EQUAL_INT(1, hlc_compare(&after_a1, &a1));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hlc_init);
    RUN_TEST(test_hlc_ztimer);
    RUN_TEST(test_hlc_get_current_timestamp);
    RUN_TEST(test_hlc_update_current_timestamp_with_later);
    RUN_TEST(test_hlc_update_current_timestamp_with_earlier);
    RUN_TEST(test_hlc_failing_time);
    RUN_TEST(test_hlc_sometimes_failing_time);
    RUN_TEST(test_complex_time_series);
    return UNITY_END();
}
