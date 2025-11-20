#include <assert.h>
#include <stdint.h>

#include "ztimer/stopwatch.h"
#include "hybrid_logical_clock.h"

#include "hlc_ztimer.h"

int hlc_ztimer_init(hlc_ztimer_t *ctx, uint32_t offset)
{
    assert(ctx != NULL);

    ztimer_stopwatch_init(ZTIMER_SEC, &ctx->stopwatch);
    ztimer_stopwatch_start(&ctx->stopwatch);
    ctx->offset = offset;
    return 0;
}

int hlc_ztimer_get_time(void *ctx, hlc_physical_t *out)
{
    assert(ctx != NULL);
    assert(out != NULL);

    hlc_ztimer_t *_ctx = (hlc_ztimer_t *) ctx;

    uint32_t now = ztimer_stopwatch_measure(&_ctx->stopwatch);
    *out = now + _ctx->offset;
    return 0;
}
