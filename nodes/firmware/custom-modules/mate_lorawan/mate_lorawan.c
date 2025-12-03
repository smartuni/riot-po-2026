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
#include "tables/records.h"
#include "tables/types.h"
#include "cbor_serialization.h"
#include "personalization.h"
#include "event.h"
#include "event/thread.h"
#if RIOT_CONFIG_DEVICE_TYPE == DEVICE_TYPE_SENSEMATE
#include "events_creation.h"
#endif
#define LOG_LEVEL   LOG_INFO
#include "log.h"

#define _LOGDBG(...) LOG_DEBUG("[LoRaWAN]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[LoRaWAN]: " __VA_ARGS__)

/* Interval between data transmissions, in seconds */
#define SEND_INTERVAL_SEC 1
/* Size of reception message queue */
#define QUEUE_SIZE 8

/* max record / serialization sizes
 * TODO: obtain from records/tables interface ? */
#define MAX_SIGNATURE_SIZE 80
#define MAX_SERIALIZED_RECORD_SIZE 128

/* Duration to trigger send_event */
//#define TIMEOUT_DURATION 60000000
#define TIMEOUT_DURATION 20000000
/* Stack for reception thread */
static char _rx_thread_stack[THREAD_STACKSIZE_MAIN];
/* Message queue for reception thread] */
static msg_t _rx_msg_queue[QUEUE_SIZE];
static table_memo_t _self_state_change_memo;
static table_query_t _self_state_change_query;
static tables_context_t *_tables;

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
 * @brief   Send a LoRaWAN packet with given payload.
 * @param   netif       Pointer to the LoRaWAN network interface.
 * @param   buf         Pointer to the data to be sent.
 * @param   len         length of buf.
 *
 * @retval   0 on success
 * @retval  -1 on failure
 */
static int _send_lorawan_packet(const netif_t *netif, const uint8_t *buf, size_t len);

/**
 * @brief   Print to STDOUT the received packet.
 * @param   pkt  Pointer to the received packet.
 */
static void _handle_received_packet(gnrc_pktsnip_t *pkt);

static void send_current_state_handler(event_t *event);
static void send_periodic_handler(event_t *event);
event_t send_periodic_event = { .handler = send_periodic_handler };
event_t send_current_state_event = { .handler = send_current_state_handler};
event_timeout_t periodic_send_timeout;

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

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
            _LOGDBG("No network interface found.\n");
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

    while (joinAttempts < 1) {
        status = NETOPT_ENABLE;
        _LOGDBG("Joining LoRaWAN network...\n");
        ztimer_now_t timeout = ztimer_now(ZTIMER_SEC);
        netif_set_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));

        while (ztimer_now(ZTIMER_SEC) - timeout < 20) {
            /* Wait for a while to allow the join process to complete */
            ztimer_sleep(ZTIMER_SEC, 2);

            netif_get_opt(netif, NETOPT_LINK, 0, &status, sizeof(status));
            if (status == NETOPT_ENABLE) {
                _LOGDBG("Joined network successfully.\n");
                /* Set the data rate */
                netif_set_opt(netif, NETOPT_LORAWAN_DR, 0, &data_rate, sizeof(data_rate));
                /* Disable uplink confirmation requests */
                status = NETOPT_DISABLE;
                netif_set_opt(netif, NETOPT_ACK_REQ, 0, &status, sizeof(status));
                _LOGDBG("Uplink confirmation requests disabled.\n");
                return 0;
            }
        }
        joinAttempts++;
    }

    return -1;
}

static int _send_lorawan_packet(const netif_t *netif, const uint8_t *buf, size_t len)
{
    assert(netif != NULL);
    _LOGDBG("Sending LoRaWAN package.\n");
    int result;
    gnrc_pktsnip_t *packet;
    gnrc_pktsnip_t *header;
    gnrc_netif_hdr_t *netif_header;
    uint8_t address = 1;
    msg_t msg;

    _LOGDBG("Package size: %d\n", len);
    //print_hex_arr(buf, len);
    packet = gnrc_pktbuf_add(NULL, buf, len, GNRC_NETTYPE_UNDEF);
    if (packet == NULL) {
        _LOGDBG("Failed to create packet.");
        return -1;
    }

    if (gnrc_neterr_reg(packet) != 0) {
        _LOGDBG("Failed to register for error reporting.");
        gnrc_pktbuf_release(packet);
        return -2;
    }

    header = gnrc_netif_hdr_build(NULL, 0, &address, sizeof(address));
    if (header == NULL) {
        _LOGDBG("Failed to create header.");
        gnrc_pktbuf_release(packet);
        return -3;
    }

    packet = gnrc_pkt_prepend(packet, header);
    netif_header = (gnrc_netif_hdr_t *)header->data;
    netif_header->flags = 0x00;

    result = gnrc_netif_send(container_of(netif, gnrc_netif_t, netif), packet);
    if (result < 1) {
        _LOGDBG("Error unable to send.\n");
        gnrc_pktbuf_release(packet);
        return -4;
    }

    /* wait for transmission confirmation */
    msg_receive(&msg);
    if (msg.type != GNRC_NETERR_MSG_TYPE) {
        _LOGDBG("Error unexpected message type %" PRIu16 ".\n", msg.type);
        return -5;
    }
    if (msg.content.value != GNRC_NETERR_SUCCESS) {
        _LOGDBG("Error unable to send, error: (%" PRIu32 ").\n", msg.content.value);
        return -6;
    }

    return 0;
}

void *rx_thread(void *arg)
{
    (void)arg;
    msg_t msg;
    /* initialize the message queue] */
    msg_init_queue(_rx_msg_queue, QUEUE_SIZE);

    /* registration entry for incoming packets */
    static gnrc_netreg_entry_t netreg_entry;

    /* register for receiving  LoRaWAN packets in our rx thread */
    gnrc_netreg_entry_init_pid(&netreg_entry,
                               GNRC_NETREG_DEMUX_CTX_ALL,
                               thread_getpid());

    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &netreg_entry);

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
    _LOGDBG("Received package from TTN.\n");
    assert(pkt != NULL);
    gnrc_pktsnip_t *snip = pkt;
    /* iterate over all packet snippets] */
    while (snip != NULL) {
        /* LoRaWAN payload will have 'undefined' type */
        if (snip->type == GNRC_NETTYPE_UNDEF) {
            _LOGDBG("%d bytes: \n", pkt->size);
            print_hex_arr(pkt->data, pkt->size);

            table_record_t record;
            table_record_data_buffer_t record_data;
            uint8_t signature[MAX_SIGNATURE_SIZE];
            size_t signature_len = sizeof(signature);

            int res = cbor_deserialize(pkt->data, pkt->size, &record,
                                        &record_data, signature, &signature_len);

            if (res) {
                _LOGDBG("cbor_deserialize failed: %d\n", res);
                break;
            }

            table_merge_result_t result;
            res = tables_merge_record(_tables, &record, &result);
            if (res) {
                _LOGDBG("tables_merge_record failed: %d\n", res);
                break;
            }
            _LOGDBG("merge result: %d\n", result.updated);

            if (result.updated || result.new) {
#if RIOT_CONFIG_DEVICE_TYPE == DEVICE_TYPE_SENSEMATE
                event_post(EVENT_PRIO_MEDIUM, &eventNews);
                _LOGDBG("Downlink received and table updated.\n");
#endif
            } else if (result.rejected_sig || result.invalid_record){
                _LOGDBG("Error updating table\n");
            } else {
                _LOGDBG("Downlink received. No updates.\n");
            }
        }
        snip = snip->next;
    }
    /* always release the packet buffer to prevent memory leaks */
    gnrc_pktbuf_release(pkt);
}

static void send_current_state_handler(event_t *event)
{
    _LOGDBG("send_current_state_handler\n");
    (void) event;
}

static char _send_record_str_buf[TABLE_RECORD_STRING_SIZE];

static int _send_record(const table_record_t *record)
{
    size_t out_len = MAX_SERIALIZED_RECORD_SIZE;
    uint8_t out_buf[out_len];

    int res = cbor_serialize_record_no_sig(record, out_buf, &out_len);
    _LOGDBG("_send_record serialize:(%d) %s\n", res, ok(res == 0));
    if (res) {
        return -1;
    }

    record_tostr(record, _send_record_str_buf,
            sizeof(_send_record_str_buf));
    _LOGINF("TX %s\n", _send_record_str_buf);

    return _send_lorawan_packet(netif, out_buf, out_len);
}

static void send_periodic_handler(event_t *event)
{
    (void)event;

    /* reset periodic timeout */
    event_timeout_set(&periodic_send_timeout, TIMEOUT_DURATION);

    TABLE_ITERATOR(iterator, _tables);

    table_record_t *record;
    table_query_t query;
    const node_id_t *writer_id = &self_node_id;

    tables_init_query(&query, RECORD_GATE_REPORT, writer_id, NULL);

    int res = tables_iterator_init(_tables, &iterator, &query);
    _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        return;
    }

    while(tables_iterator_next(_tables, &iterator, &record, NULL, NULL) == 0) {
        res = _send_record(record);
        _LOGDBG("send_periodic_handler _send_record: %d\n", res);
    }

    tables_init_query(&query, RECORD_GATE_OBSERVATION, writer_id, NULL);

    res = tables_iterator_init(_tables, &iterator, &query);
    _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        return;
    }

    while(tables_iterator_next(_tables, &iterator, &record, NULL, NULL) == 0) {
        res = _send_record(record);
        _LOGDBG("send_periodic_handler _send_record: %d\n", res);
    }
}

static void _table_self_state_updated_cb(tables_context_t *ctx, const table_record_t *record,
                                         const table_query_t *query, void *arg)
{
    (void)ctx;
    (void)arg;
    (void)query;
    (void)record;

    _LOGDBG("%s\n", __func__);

    /* For now just offload the event to periodic event context baceause the send function
     * cannot be safely called from any thread-context (but the callback will be executed in
     * the thread that modified the record).
     * Alternatively this function could copy the record to an offloaded queue instead. */
    event_post(EVENT_PRIO_MEDIUM, &send_periodic_event);
}

int mate_lorawan_start(tables_context_t *t)
{
    _LOGDBG("Starting module.\n");
    _tables = t;

    /* find the LoRaWAN network interface and connect */
    netif = _find_lorawan_network_interface();
    if (netif == NULL) {
        _LOGINF("No network interface found.\n");
        return -1;
    }
    if(_join_lorawan_network(netif) == -1){
        _LOGINF("Join failed.\n");
        return -1;
    } else {
        _LOGINF("Joined.\n");
    }

    _LOGDBG("Starting receive thread.\n");
    /* create the reception thread] */
    kernel_pid_t rx_pid = thread_create(_rx_thread_stack, sizeof(_rx_thread_stack),
                                        THREAD_PRIORITY_MAIN + 1,
                                        THREAD_CREATE_STACKTEST, rx_thread, NULL,
                                        "lorawan_rx");
    if (-EINVAL == rx_pid) {
        _LOGDBG("Failed to create reception thread.\n");
        return -1;
    }else{
        _LOGDBG("Receive thread started successfully.\n");
    }

    /* Init timeout event */
    event_timeout_init(&periodic_send_timeout, EVENT_PRIO_MEDIUM, (event_t*)&send_periodic_event);
    event_timeout_set(&periodic_send_timeout, TIMEOUT_DURATION);

    ///* send the first uplink right away so that the device shows up fast */
    //event_post(EVENT_PRIO_HIGHEST, &send_event_timeout);

    void *cb_arg = 0;

    /* register for changes in the table. In this case we only care about gate report updates
     * written by this node itself. No involed Id is needed. */
    const node_id_t *writer_id = &self_node_id;
    const node_id_t *involved_id = NULL;
    tables_init_query(&_self_state_change_query, RECORD_UNDEFINED, writer_id, involved_id);
    tables_add_memo(_tables, &_self_state_change_memo, &_self_state_change_query,
                    _table_self_state_updated_cb, cb_arg);

    _LOGDBG("Start up succesful.\n");
    return 0;
}


