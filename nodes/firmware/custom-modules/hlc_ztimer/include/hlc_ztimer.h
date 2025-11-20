#pragma once

#include <stdint.h>

#include "ztimer/stopwatch.h"
#include "hybrid_logical_clock.h"

/**
 * @brief Context for an HLC ztimer physical time provider.
 */
typedef struct {
    ztimer_stopwatch_t stopwatch; /**< ztimer stopwatch instance */
    uint32_t offset;              /**< Time offset provided by the user */
} hlc_ztimer_t;

/**
 * @brief Initialize an HLC ztimer physical time provider with a given @p offset.
 *
 * @param ctx       HLC ztimer physical time provider context
 * @param offset    Initial offset in seconds to apply to the time
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int hlc_ztimer_init(hlc_ztimer_t *ctx, uint32_t offset);

/**
 * @brief Get the HLC physical time. This implements the callback interface of the HLC
 *        module.
 *
 * @param ctx       Initialized HLC physical time provider context
 * @param out       Pointer to the HLC physical time to set
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int hlc_ztimer_get_time(void *ctx, hlc_physical_t *out);
