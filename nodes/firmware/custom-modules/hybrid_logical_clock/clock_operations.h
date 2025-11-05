#pragma once

#include "hybrid_logical_clock.h"

/**
 * @brief Compare two HLC physical timestamps
 *
 * @param p1 Pointer to the first physical timestamp
 * @param p2 Pointer to the second physical timestamp
 *
 * @retval -1 if @p p1 < @p p2
 * @retval 1 if @p p1 > @p p2
 * @retval 0 if @p p1 == @p p2
 */
int _hlc_compare_physical(const hlc_physical_t *p1, const hlc_physical_t *p2);

/**
 * @brief Compare two HLC logical timestamps
 *
 * @param l1 Pointer to the first logical timestamp
 * @param l2 Pointer to the second logical timestamp
 *
 * @retval -1 if @p l1 < @p l2
 * @retval 1 if @p l1 > @p l2
 * @retval 0 if @p l1 == @p l2
 */
int _hlc_compare_logical(const hlc_logical_t *l1, const hlc_logical_t *l2);

/**
 * @brief Get the maximum of two HLC physical timestamps
 *
 * @param p1    Pointer to the first physical timestamp
 * @param p2    Pointer to the second physical timestamp
 * @param max_p Pointer to store the resulting maximum physical timestamp
 */
void _hlc_max_physical(const hlc_physical_t *p1, const hlc_physical_t *p2, hlc_physical_t *max_p);

/**
 * @brief Get the maximum of two HLC logical timestamps
 *
 * @param l1    Pointer to the first logical timestamp
 * @param l2    Pointer to the second logical timestamp
 * @param max_l Pointer to store the resulting maximum logical timestamp
 */
void _hlc_max_logical(const hlc_logical_t *l1, const hlc_logical_t *l2, hlc_logical_t *max_l);
