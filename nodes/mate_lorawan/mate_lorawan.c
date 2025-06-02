/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     module_mate_lorawan
 * @{
 *
 * @file
 * @brief       mate_lorawan implementation
 *
 * @author      Paul Lange <paul.lange2@haw-hamburg.de>
 *
 * @}
 */

#include "mate_lorawan.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"

#include "net/netdev.h"
#include "net/netif.h"

#include "net/gnrc/pktbuf.h"
#include "net/gnrc/pkt.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/netif/hdr.h"

#include "saul_reg.h"
#include "phydat.h"

#include "od.h"
#include "msg.h"
#include "thread.h"

#include "ztimer.h"

#include "cbor.h"

/* Interval between data transmissions, in seconds */
#define SEND_INTERVAL_SEC 1

/* Size of reception message queue */
#define QUEUE_SIZE 8

/* Stack for reception thread */
static char _rx_thread_stack[THREAD_STACKSIZE_DEFAULT];

/* [TASK 3.3: Message queue for reception thread] */
static msg_t _rx_msg_queue[QUEUE_SIZE];

static msg_t _tx_msg_queue[QUEUE_SIZE];

uint8_t send_buffer[128];
size_t buffer_size;

/**
 * @brief   Find the LoRaWAN network interface in the registry.
 * @return Pointer to the LoRaWAN network interface, or NULL if not found.
 */
static netif_t *_find_lorawan_network_interface(void);

/**
 * @brief   Join the LoRaWAN network using OTAA.
 * @param   netif  Pointer to the LoRaWAN network interface.
 *
 * This function will attempt to join the LoRaWAN network using Over-The-Air
 * Activation (OTAA). It will keep retrying until a successful join is achieved.
 */
static void _join_lorawan_network(const netif_t *netif);

/**
 * @brief   Send a LoRaWAN packet with temperature data.
 * @param   netif       Pointer to the LoRaWAN network interface.
 * @param   temperature Pointer to the temperature data to be sent.
 *
 * @retval   0 on success
 * @retval  -1 on failure
 */
static int _send_lorawan_packet(const netif_t *netif);

/**
 * @brief   Print to STDOUT the received packet.
 * @param   pkt  Pointer to the received packet.
 */
static void _print_received_packet(gnrc_pktsnip_t *pkt);

static void createMessage(void);

static netif_t *_find_lorawan_network_interface(void)
{
    /* [TASK 2.1: implement function to identify lorawan interface here]*/
    netif_t *netif = NULL;
    uint16_t device_type = 0;

    do {
        netif = netif_iter(netif);
        if (netif == NULL) {
            puts("No network interface found");
            break;
        }
        netif_get_opt(netif, NETOPT_DEVICE_TYPE, 0, &device_type, sizeof(device_type));
    } while (device_type != NETDEV_TYPE_LORA);

    return netif;
}

static void _join_lorawan_network(const netif_t *netif)
{
    assert(netif != NULL);

    /* [TASK 2.2: implement join function here ]*/
    netopt_enable_t status;
    uint8_t data_rate = 5;

    while (1) {
        status = NETOPT_ENABLE;
        printf("Joining LoRaWAN network...\n");
        ztimer_now_t timeout = ztimer_now(ZTIMER_SEC);
        netif_set_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));

        while (ztimer_now(ZTIMER_SEC) - timeout < 10000) {
            /* Wait for a while to allow the join process to complete */
            ztimer_sleep(ZTIMER_SEC, 1);

            netif_get_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));
            if (status == NETOPT_ENABLE) {
                printf("Joined LoRaWAN network successfully\n");

                /* Set the data rate */
                netif_set_opt(netif, NETOPT_LORAWAN_DR, 0, &data_rate, sizeof(data_rate));

                /* Disable uplink confirmation requests */
                status = NETOPT_DISABLE;
                netif_set_opt(netif, NETOPT_ACK_REQ, 0, &status, sizeof(status));
                return;
            }
        }
    }
}

static int _send_lorawan_packet(const netif_t *netif)
{
    assert(netif != NULL);

    int result;
    gnrc_pktsnip_t *packet;
    gnrc_pktsnip_t *header;
    gnrc_netif_hdr_t *netif_header;
    uint8_t address = 1;
    msg_t msg;
    //uint8_t data[2];

    packet = gnrc_pktbuf_add(NULL, send_buffer, 8, GNRC_NETTYPE_UNDEF);
    if (packet == NULL) {
        puts("Failed to create packet");
        return -1;
    }

    if (gnrc_neterr_reg(packet) != 0) {
        puts("Failed to register for error reporting");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    header = gnrc_netif_hdr_build(NULL, 0, &address, sizeof(address));
    if (header == NULL) {
        puts("Failed to create header");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    packet = gnrc_pkt_prepend(packet, header);
    netif_header = (gnrc_netif_hdr_t *)header->data;
    netif_header->flags = 0x00;

    result = gnrc_netif_send(container_of(netif, gnrc_netif_t, netif), packet);
    if (result < 1) {
        printf("error: unable to send\n");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    /* wait for transmission confirmation */
    msg_receive(&msg);
    if (msg.type != GNRC_NETERR_MSG_TYPE) {
        printf("error: unexpected message type %" PRIu16 "\n", msg.type);
        return -1;
    }
    if (msg.content.value != GNRC_NETERR_SUCCESS) {
        printf("error: unable to send, error: (%" PRIu32 ")\n", msg.content.value);
        return -1;
    }

    for (size_t i = 0; i < 8; i++) {
            printf("%02X ", send_buffer[i]);
        }
    printf("\n");

    return 0;
}

void *rx_thread(void *arg)
{
    (void)arg;
    msg_t msg;

    /* [TASK 3.4: initialize the message queue] */
    msg_init_queue(_rx_msg_queue, QUEUE_SIZE);

    while (1) {
        /* [TASK 3.5: wait until we get a message]*/
        msg_receive(&msg);

        if (msg.type == GNRC_NETAPI_MSG_TYPE_RCV) {
            puts("Received data");
            gnrc_pktsnip_t *pkt = msg.content.ptr;
            _print_received_packet(pkt);
        }
    }

    /* never reached */
    return NULL;
}

static void _print_received_packet(gnrc_pktsnip_t *pkt)
{
    assert(pkt != NULL);

    gnrc_pktsnip_t *snip = pkt;

    /* [TASK 3.7: iterate over all packet snippets] */
    while (snip != NULL) {
        /* LoRaWAN payload will have 'undefined' type */
        if (snip->type == GNRC_NETTYPE_UNDEF) {
            od_hex_dump(((uint8_t *)pkt->data), pkt->size, OD_WIDTH_DEFAULT);
        }
        snip = snip->next;
    }

    /* always release the packet buffer to prevent memory leaks */
    gnrc_pktbuf_release(pkt);
}

static void createMessage(void){
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, send_buffer, sizeof(send_buffer), 0);

    cbor_encoder_create_map(&encoder, &mapEncoder, 2);

    cbor_encode_int(&mapEncoder, 0);
    cbor_encode_int(&mapEncoder, 1234);

    cbor_encode_int(&mapEncoder, 1);
    cbor_encode_int(&mapEncoder, 0);

    buffer_size = cbor_encoder_get_buffer_size(&encoder, send_buffer);
    
    cbor_encoder_close_container(&encoder, &mapEncoder);
}

int start_lorawan(void)
{
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_CPU);

    int result;
    netif_t *netif = NULL;

    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);

    /* initialize message queue */
    msg_init_queue(_tx_msg_queue, QUEUE_SIZE);

    /* find the LoRaWAN network interface and connect */
    netif = _find_lorawan_network_interface();
    if (netif == NULL) {
        puts("No LoRaWAN network interface found");
        return -1;
    }

    _join_lorawan_network(netif);

    /* [TASK 3.1: create the reception thread] */
    kernel_pid_t rx_pid = thread_create(_rx_thread_stack, sizeof(_rx_thread_stack),
                                        THREAD_PRIORITY_MAIN - 1,
                                        THREAD_CREATE_STACKTEST, rx_thread, NULL,
                                        "lorawan_rx");
    if (-EINVAL == rx_pid) {
        puts("Failed to create reception thread");
        return -1;
    }

    /* [TASK 3.2: receive LoRaWAN packets in our reception thread] */
    gnrc_netreg_entry_t entry = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                           rx_pid);
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &entry);

    /* record the starting time */
    ztimer_now_t last_wakeup = ztimer_now(ZTIMER_SEC);

    while (1) {
        createMessage();
        /* [TASK 2.3: send sensor data via LoRaWAN ] */
        puts("Sending data...");
        result = _send_lorawan_packet(netif);
        if (result != 0) {
            puts("Failed to send LoRaWAN packet");
        } else {
            printf("Sent LoRaWAN packet successfully\n");
        }

        /* wait a bit */
        printf("Waiting for %d seconds...\n", SEND_INTERVAL_SEC);
        ztimer_periodic_wakeup(ZTIMER_SEC, &last_wakeup, SEND_INTERVAL_SEC);
    }

    return 0;
}