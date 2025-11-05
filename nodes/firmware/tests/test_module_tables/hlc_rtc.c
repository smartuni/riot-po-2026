#include <time.h>

#include "unity.h"
#include "periph/rtc.h"
#include "hybrid_logical_clock.h"

int hlc_rtc_get_time(hlc_physical_t *out)
{
    TEST_ASSERT_NOT_NULL(out);
    struct tm time;
    int result = rtc_get_time(&time);
    if (result != 0) {
        return HLC_ERROR_GET_TIME;
    }

    *out = (hlc_physical_t)rtc_mktime(&time);
    return 0;
}
