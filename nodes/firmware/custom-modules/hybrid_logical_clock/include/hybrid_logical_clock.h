/**
 * @brief       Public interface of the Hybrid Logical Clock (HLC) module
 *
 * @author      Leandro Lanzieri
 */

#pragma once

#include <stdint.h>

#include "mutex.h"

/**
 * @brief Hybrid Logical Clock physical timestamp type
 */
typedef uint32_t hlc_physical_t;

/**
 * @brief Hybrid Logical Clock logical timestamp type
 */
typedef uint32_t hlc_logical_t;

/**
 * @brief Hybrid Logical Clock timestamp type
 */
typedef struct {
    hlc_physical_t physical; /**< Physical timestamp */
    hlc_logical_t logical;    /**< Logical timestamp */
} hlc_timestamp_t;

#define HLC_ERROR_GET_TIME   -2  /**< Error code for time retrieval failure */

/**
 * @brief Function type for getting the current physical time.
 * @param out Pointer to store the current physical time
 *
 * @retval 0 on success
 * @retval HLC_ERROR_GET_TIME if an error occurred when getting the time
 */
typedef int (*hlc_get_physical_time_fn)(hlc_physical_t *out);

/**
 * @brief Hybrid Logical Clock context structure
 */
typedef struct {
    mutex_t lock;              /**< Mutex to protect access to the local HLC */
    hlc_timestamp_t local_hlc;   /**< Local HLC timestamp */

    /**< Function to get current physical time */
    hlc_get_physical_time_fn get_physical_time;
} hlc_ctx_t;

/**
 * @brief Initialize a Hybrid Logical Clock context
 *
 * @retval 0 on success
 * @retval HLC_ERROR_GET_TIME if an error occurred when getting the current time
 */
int hlc_init(hlc_ctx_t *ctx, hlc_get_physical_time_fn get_time_fn);

/**
 * @brief Get the current HLC timestamp
 *
 * This function will retrieve the current HLC timestamp, updating the local
 * HLC state based on the current RTC time.
 *
 * @param ctx Pointer to the HLC context
 * @param out Pointer to store the current HLC timestamp
 *
 * @retval 0 on success
 * @retval HLC_ERROR_GET_TIME if an error occurred when getting the current time
 */
int hlc_get_current_timestamp(hlc_ctx_t *ctx, hlc_timestamp_t *out);

/**
 * @brief Compare two HLC timestamps
 *
 * @param ts1 First HLC timestamp
 * @param ts2 Second HLC timestamp
 *
 * @retval -1 if @p ts1 < @p ts2
 * @retval 1 if @p ts1 > @p ts2
 * @retval 0 if @p ts1 == @p ts2
 */
int hlc_compare(const hlc_timestamp_t *ts1, const hlc_timestamp_t *ts2);

/**
 * @brief Update the local HLC timestamp with a received remote timestamp
 *
 * This function updates the local HLC timestamp based on the provided remote
 * timestamp and the current RTC time.
 *
 * @param ctx Pointer to the HLC context
 * @param ts  Pointer to the received remote HLC timestamp
 * @param out Pointer to store the updated local HLC timestamp. Can be NULL.
 *
 * @retval 0 on success
 * @retval HLC_ERROR_GET_TIME if an error occurred when getting the current time
 */
int hlc_update_with_remote_timestamp(hlc_ctx_t *ctx, const hlc_timestamp_t *ts, hlc_timestamp_t *out);
