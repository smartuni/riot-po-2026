/**
 * @brief      Implementation of the Hybrid Logical Clock (HLC) module
 *
 * @author     Leandro Lanzieri
 */
#include <assert.h>

#include "mutex.h"
#include "hybrid_logical_clock.h"
#include "clock_operations.h"

int hlc_init(hlc_ctx_t *ctx, hlc_get_physical_time_fn get_time_fn, void *arg)
{
    assert(ctx != NULL);
    assert(get_time_fn != NULL);

    int result = 0;

    ctx->get_physical_time = get_time_fn;
    ctx->get_physical_time_arg = arg;
    mutex_init(&ctx->lock);
    mutex_lock(&ctx->lock);

    // TODO: persistent storage restore?
    result = ctx->get_physical_time(ctx->get_physical_time_arg,
                                    &ctx->local_hlc.physical);
    if (result != 0) {
        result = HLC_ERROR_GET_TIME;
        goto free_and_exit;
    }

    ctx->local_hlc.logical  = 0;

free_and_exit:
    mutex_unlock(&ctx->lock);
    return result;
}

int hlc_compare(const hlc_timestamp_t *ts1, const hlc_timestamp_t *ts2)
{
    assert(ts1 != NULL);
    assert(ts2 != NULL);

    int result = _hlc_compare_physical(&ts1->physical, &ts2->physical);
    if (result != 0) {
        return result;
    }
    return _hlc_compare_logical(&ts1->logical, &ts2->logical);
}

int hlc_update_with_remote_timestamp(hlc_ctx_t *ctx, const hlc_timestamp_t *ts, hlc_timestamp_t *out)
{
    assert(ctx != NULL);
    assert(ts != NULL);

    int result = 0;
    hlc_physical_t now_physical;

    mutex_lock(&ctx->lock);

    // get p_now (physical now)
    result = ctx->get_physical_time(ctx->get_physical_time_arg, &now_physical);
    if (result != 0) {
        result = HLC_ERROR_GET_TIME;
        goto free_and_exit;
    }

    // p_now > p_local && p_now > p_remote -> p_local = p_now; l_local = 0
    if (_hlc_compare_physical(&now_physical, &ctx->local_hlc.physical) > 0 &&
        _hlc_compare_physical(&now_physical, &ts->physical) > 0) {
        ctx->local_hlc.physical = now_physical;
        ctx->local_hlc.logical  = 0;

    // elif p_local == p_remote -> l_local = max(l_local, l_remote) + 1
    } else if (_hlc_compare_physical(&ctx->local_hlc.physical, &ts->physical) == 0) {
        hlc_logical_t new_logical;
        _hlc_max_logical(&ctx->local_hlc.logical, &ts->logical, &new_logical);
        new_logical++;
        ctx->local_hlc.logical = new_logical;

    // elif p_local > p_remote -> l_local = l_local + 1
    } else if (_hlc_compare_physical(&ctx->local_hlc.physical, &ts->physical) > 0) {
        ctx->local_hlc.logical++;
    }

    // else (p_remote > p_local) -> p_local = p_remote; l_local = l_remote + 1
    else {
        ctx->local_hlc.physical = ts->physical;
        ctx->local_hlc.logical  = ts->logical + 1;
    }

    if (out != NULL) {
        *out = ctx->local_hlc;
    }

free_and_exit:
    mutex_unlock(&ctx->lock);
    return result;
}

int hlc_get_current_timestamp(hlc_ctx_t *ctx, hlc_timestamp_t *out)
{
    assert(ctx != NULL);
    assert(out != NULL);

    int result = 0;
    hlc_physical_t current_physical;

    mutex_lock(&ctx->lock);

    result = ctx->get_physical_time(ctx->get_physical_time_arg, &current_physical);
    if (result != 0) {
        result = HLC_ERROR_GET_TIME;
        goto free_and_exit;
    }

    if (_hlc_compare_physical(&current_physical, &ctx->local_hlc.physical) > 0) {
        ctx->local_hlc.physical = current_physical;
        ctx->local_hlc.logical  = 0;
    } else {
        ctx->local_hlc.logical++;
    }

    *out = ctx->local_hlc;

free_and_exit:
    mutex_unlock(&ctx->lock);
    return result;
}
