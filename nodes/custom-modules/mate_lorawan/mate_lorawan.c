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
 * @brief       Lorawan implementation for SenseMate / GateMate
 *
 * @author      Paul Lange <paul.lange2@haw-hamburg.de>
 * @author      Jannik Schön <jannik.schoen@haw-hamburg.de>
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
#include "event/timeout.h"
#include "cbor.h"
#include "tables.h"
#include "event.h"
#include "event/thread.h"
#if DEVICE_TYPE == 1
#include "events_creation.h"
#endif

/* Interval between data transmissions, in seconds */
#define SEND_INTERVAL_SEC 1
/* Size of reception message queue */
#define QUEUE_SIZE 8
/* Max length of serialized data to send to backend server */
#define MAX_SEND_BUFFER_SIZE (MAX_GATE_COUNT * (sizeof(is_state_entry)))
/* Size of single LoRaWAN message */
#define SEND_BUFFER_SIZE 50
#define MAX_RECEIVE_SIZE 222
/* Duration to trigger send_event */
#define TIMEOUT_DURATION 60000000
/* Stack for reception thread */
static char _rx_thread_stack[THREAD_STACKSIZE_DEFAULT];
/* Message queue for reception thread] */
static msg_t _rx_msg_queue[QUEUE_SIZE];

/* Buffer to manage serialized send data */
cbor_buffer cbor_send_buffer;
uint8_t send_buffer[MAX_SEND_BUFFER_SIZE];
uint8_t msg_sizes[MAX_GATE_COUNT];

cbor_buffer cbor_receive_buffer;

//event_queue_t lorawan_queue;
event_timeout_t event_timeout;
netif_t *netif = NULL;

/**
 * @brief   Find the LoRaWAN network interface in the registry.
 * @return Pointer to the LoRaWAN network interface, or NULL if not found.
 */
static netif_t *_find_lorawan_network_interface(void);

/**
 * @brief   Join the LoRaWAN network using OTAA.
 * @param   netif  Pointer to the LoRaWAN network interface.
 *
 * @retval   0 on success
 * @retval  -1 on failure
 *
 * This function will attempt to join the LoRaWAN network using Over-The-Air
 * Activation (OTAA). It will keep retrying until a successful join is achieved.
 */
static int _join_lorawan_network(const netif_t *netif);

/**
 * @brief   Send a LoRaWAN packet with temperature data.
 * @param   netif       Pointer to the LoRaWAN network interface.
 * @param   temperature Pointer to the temperature data to be sent.
 *
 * @retval   0 on success
 * @retval  -1 on failure
 */
static int _send_lorawan_packet(const netif_t *netif, int msg_no, int read);

/**
 * @brief   Print to STDOUT the received packet.
 * @param   pkt  Pointer to the received packet.
 */
static void _handle_received_packet(gnrc_pktsnip_t *pkt);

static void send_handler_is_state_table(event_t *event);
static void send_handler_seen_status_table(event_t *event);
static void send_handler_timeout(event_t *event);

event_t send_event_timeout = { .handler = send_handler_timeout };
event_t send_seen_status_table = { .handler = send_handler_seen_status_table};
event_t send_is_state_table = { .handler = send_handler_is_state_table};
// event_t send_target_state_table = { .handler = send_handler};
// event_t send_jobs_table = { .handler = send_handler};
// event_t send_timestamp_table = { .handler = send_handler};
// event_t send_event_timeout = { .handler = send_handler_timeout };

static void print_hex_arr(const uint8_t *data, unsigned len)
{
    printf("{");
    for (unsigned i = 0; i < len; i++) {
        printf(" 0x%02x%c", data[i], (i == len-1) ? ' ' : ',');
    }
    printf("}\n");
}

static netif_t *_find_lorawan_network_interface(void)
{
    netif_t *netif = NULL;
    uint16_t device_type = 0;

    do {
        netif = netif_iter(netif);
        if (netif == NULL) {
            puts("[LoRaWAN]: No network interface found.");
            break;
        }
        netif_get_opt(netif, NETOPT_DEVICE_TYPE, 0, &device_type, sizeof(device_type));
    } while (device_type != NETDEV_TYPE_LORA);

    return netif;
}

static int _join_lorawan_network(const netif_t *netif)
{
    assert(netif != NULL);
    netopt_enable_t status;
    uint8_t data_rate = 5;
    int joinAttempts = 0;

    while (joinAttempts < 5) {
        status = NETOPT_ENABLE;
        printf("[LoRaWAN]: Joining LoRaWAN network...\n");
        ztimer_now_t timeout = ztimer_now(ZTIMER_SEC);
        netif_set_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));

        while (ztimer_now(ZTIMER_SEC) - timeout < 1000) {
            /* Wait for a while to allow the join process to complete */
            ztimer_sleep(ZTIMER_SEC, 2);

            netif_get_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));
            if (status == NETOPT_ENABLE) {
                printf("[LoRaWAN]: Joined network successfully.\n");
                /* Set the data rate */
                netif_set_opt(netif, NETOPT_LORAWAN_DR, 0, &data_rate, sizeof(data_rate));
                /* Disable uplink confirmation requests */
                status = NETOPT_DISABLE;
                netif_set_opt(netif, NETOPT_ACK_REQ, 0, &status, sizeof(status));
                printf("[LoRaWAN]: Uplink confirmation requests disabled.\n");
                return 0;
            }
        }
        joinAttempts++;
    }
    
    return -1;
}

static int _send_lorawan_packet(const netif_t *netif, int msg_no, int read)
{
    assert(netif != NULL);
    printf("[LoRaWAN]: Sending LoRaWAN package.\n");
    int result;
    gnrc_pktsnip_t *packet;
    gnrc_pktsnip_t *header;
    gnrc_netif_hdr_t *netif_header;
    uint8_t address = 1;
    msg_t msg;

    printf("[LoRaWAN]: Package size: %d\n", cbor_send_buffer.package_size[msg_no]);
    print_hex_arr(cbor_send_buffer.buffer + read, cbor_send_buffer.package_size[msg_no]);
    packet = gnrc_pktbuf_add(NULL, cbor_send_buffer.buffer + read, cbor_send_buffer.package_size[msg_no], GNRC_NETTYPE_UNDEF);
    if (packet == NULL) {
        puts("[LoRaWAN]: Failed to create packet.");
        return -1;
    }

    if (gnrc_neterr_reg(packet) != 0) {
        puts("[LoRaWAN]: Failed to register for error reporting.");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    header = gnrc_netif_hdr_build(NULL, 0, &address, sizeof(address));
    if (header == NULL) {
        puts("[LoRaWAN]: Failed to create header.");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    packet = gnrc_pkt_prepend(packet, header);
    netif_header = (gnrc_netif_hdr_t *)header->data;
    netif_header->flags = 0x00;

    result = gnrc_netif_send(container_of(netif, gnrc_netif_t, netif), packet);
    if (result < 1) {
        printf("[LoRaWAN]: Error unable to send.\n");
        gnrc_pktbuf_release(packet);
        return -1;
    }

    /* wait for transmission confirmation */
    msg_receive(&msg);
    if (msg.type != GNRC_NETERR_MSG_TYPE) {
        printf("[LoRaWAN]: Error unexpected message type %" PRIu16 ".\n", msg.type);
        return -1;
    }
    if (msg.content.value != GNRC_NETERR_SUCCESS) {
        printf("[LoRaWAN]: Error unable to send, error: (%" PRIu32 ").\n", msg.content.value);
        return -1;
    }

    return 0;
}

void *rx_thread(void *arg)
{
    (void)arg;
    msg_t msg;
    /* initialize the message queue] */
    msg_init_queue(_rx_msg_queue, QUEUE_SIZE);
    while (1) {
        /* wait until we get a message]*/
        msg_receive(&msg);

        if (msg.type == GNRC_NETAPI_MSG_TYPE_RCV) {
            gnrc_pktsnip_t *pkt = msg.content.ptr;
            _handle_received_packet(pkt);
        }
    }
    /* never reached */
    return NULL;
}

static void _handle_received_packet(gnrc_pktsnip_t *pkt)
{
    printf("[LoRaWAN]: Received package from TTN.\n");
    assert(pkt != NULL);
    gnrc_pktsnip_t *snip = pkt;
    cbor_buffer received_buffer;
    uint8_t received_buffer_array[MAX_RECEIVE_SIZE];
    received_buffer.buffer = received_buffer_array;
    uint8_t received_buffer_size_array[MAX_GATE_COUNT];
    received_buffer.package_size = received_buffer_size_array;
    /* iterate over all packet snippets] */
    while (snip != NULL) {
        /* LoRaWAN payload will have 'undefined' type */
        if (snip->type == GNRC_NETTYPE_UNDEF) {
            received_buffer.buffer = pkt->data;
            received_buffer.cbor_size = pkt->size; 
            received_buffer.package_size[0] = pkt->size;
            print_hex_arr(received_buffer.buffer,received_buffer.package_size[0]);
            if (cbor_to_table_test(&received_buffer, 0) == 0) {
#if DEVICE_TYPE == 1
                    
                        event_post(EVENT_PRIO_HIGHEST, &eventNews);
#endif
                printf("[LoRaWAN]: Downlink received and table updated.\n");
            }else{
                printf("[LoRaWAN]: Error updating table.\n");
            }
        }
        snip = snip->next;
    }
    /* always release the packet buffer to prevent memory leaks */
    gnrc_pktbuf_release(pkt);
}

static void send_handler_timeout(event_t *event){
    (void) event;
    event_timeout_set(&event_timeout, TIMEOUT_DURATION); // reset timer
    event_post(EVENT_PRIO_HIGHEST, &send_is_state_table);
    event_post(EVENT_PRIO_HIGHEST, &send_seen_status_table);
}

static void send_handler_is_state_table(event_t *event){
    (void) event;
    int pkg_count = is_state_table_to_cbor_many_to_server(SEND_BUFFER_SIZE, &cbor_send_buffer);
    if (pkg_count == 0){
        printf("[LoRaWAN]: Nothing to send.\n");
        return;
    }
    int read = 0;
    printf("[LoRaWAN]: Sending %d packages ...\n", pkg_count);
    puts("");
    int result = 0;
    for (int msg_no = 0; msg_no < pkg_count; msg_no++){
        result = _send_lorawan_packet(netif, msg_no, read);
        if (result != 0) {
            puts("[LoRaWAN]: Failed to send packet.");
        } else {
            printf("[LoRaWAN]: Sent packet successfully.\n");
        }
        read += cbor_send_buffer.package_size[msg_no];
    }
}

static void send_handler_seen_status_table(event_t *event){
    (void) event;
    int pkg_count = seen_status_table_to_cbor_many_to_server(SEND_BUFFER_SIZE, &cbor_send_buffer);
    if (pkg_count == 0){
        printf("[LoRaWAN]: Nothing to send.\n");
        return;
    }
    int read = 0;
    printf("[LoRaWAN]: Sending %d packages ...\n", pkg_count);
    puts("");
    int result = 0;
    for (int msg_no = 0; msg_no < pkg_count; msg_no++){
        result = _send_lorawan_packet(netif, msg_no, read);
        if (result != 0) {
            puts("[LoRaWAN]: Failed to send packet.");
        } else {
            printf("[LoRaWAN]: Sent packet successfully.\n");
        }
        read += cbor_send_buffer.package_size[msg_no];
    }
}

int start_lorawan(void)
{
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    printf("[LoRaWAN]: Starting module.\n");
    //event_queue_init(&lorawan_queue);
    
    cbor_send_buffer.buffer = send_buffer;
    cbor_send_buffer.package_size = msg_sizes;

    void event_timeout_init(event_timeout_t *event_timeout, event_queue_t *queue, event_t *event);
    /* Init timeout event */
    event_timeout_init(&event_timeout, EVENT_PRIO_HIGHEST, (event_t*)&send_event_timeout);
    event_timeout_set(&event_timeout, TIMEOUT_DURATION);
    
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);

    /* find the LoRaWAN network interface and connect */
    netif = _find_lorawan_network_interface();
    if (netif == NULL) {
        puts("[LoRaWAN]: No network interface found.");
        return -1;
    }
    if(_join_lorawan_network(netif) == -1){
        printf("[LoRaWAN]: Joining LoRaWAN failed.");
        return -1;
    }

    printf("[LoRaWAN]: Starting receive thread.\n");
    /* create the reception thread] */
    kernel_pid_t rx_pid = thread_create(_rx_thread_stack, sizeof(_rx_thread_stack),
                                        THREAD_PRIORITY_MAIN - 1,
                                        THREAD_CREATE_STACKTEST, rx_thread, NULL,
                                        "lorawan_rx");
    if (-EINVAL == rx_pid) {
        puts("[LoRaWAN]: Failed to create reception thread.");
        return -1;
    }else{
        printf("[LoRaWAN]: Receive thread started successfully.\n");
    }

    /* receive LoRaWAN packets in our reception thread] */
    gnrc_netreg_entry_t entry = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL, rx_pid);
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &entry);
    printf("[LoRaWAN]: Start up succesful.\n");
    return 0;
}


