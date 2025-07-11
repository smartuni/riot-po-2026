/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_mate_lorawan mate_lorawan
 * @ingroup     sys
* @{
 * @brief       Lorawan interface for SenseMate / GateMate
 *
 * @author      Paul Lange <paul.lange2@haw-hamburg.de>
 * @author      Jannik Schön <jannik.schoen@haw-hamburg.de>
 * @}
 */

#ifndef MATE_LORAWAN_H
#define MATE_LORAWAN_H

#include "event.h"

/**
 * @brief   Intializes the LoRaWAN module and starts the event loop.
 * @retval  0 on success
 * @retval  -1 on failure
 */
int start_lorawan(void);

extern event_t send_event; // Generic event for sending data
extern event_t send_is_state_table; // Event for sending is state table
extern event_t send_target_state_table; // Event for sending target state table
extern event_t send_seen_status_table; // Event for sending seen status table
extern event_t send_jobs_table; // Event for sending jobs table
extern event_t send_timestamp_table; // Event for sending timestamp table

#endif /* MATE_LORAWAN_H */
/** @} */
