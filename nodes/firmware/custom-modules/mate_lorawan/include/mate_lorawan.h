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
#include "tables/types.h"

#define MATE_LORAWAN_TX_QUERY_MATCHES_MSG_TYPE (0x66)

/**
 * @brief   Intializes the LoRaWAN module and starts the event loop.
 *
 * @param   tables     Reference to sucessfully initialized tables instance
 *
 * @retval  0 on success
 * @retval  -1 on failure
 */
int mate_lorawan_start(tables_context_t *tables);

/**
 * @brief   Check if joining lorawan network finished sucessfully.
 *
 * @retval  true if the node joined
 * @retval  false if the join did not finish yet
 */
bool mate_lorawan_joined(void);

#endif /* MATE_LORAWAN_H */
/** @} */

