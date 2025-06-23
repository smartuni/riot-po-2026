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
 * @brief       Lorawan for senemate / gatemate
 *
 * @{
 *
 * @file
 *
 * @author      Paul Lange <paul.lange2@haw-hamburg.de>
 */

#ifndef MATE_LORAWAN_H
#define MATE_LORAWAN_H

#include "event.h"

/**
 * @brief       
 *
 * @author      
 */


int start_lorawan(void);

extern event_queue_t lorawan_queue;

extern event_t send_event;


#endif /* MATE_LORAWAN_H */
/** @} */
