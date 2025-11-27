#include "mate_ble.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_intsup.h>

#include "od.h"
#include "event.h"
#include "event/timeout.h"
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "nimble_scanner.h"
#include "nimble_addr.h"
#include "nimble/ble.h"
#include "net/bluetil/ad.h"
#include "shell.h"
#include "tables.h"
#include "tables/records.h"
#include "tables/types.h"
#include "timex.h"
#include "ztimer.h"
#include "cbor.h"
#include "cose-service.h"
#include "cbor_serialization.h"
#include "personalization.h"
#define LOG_LEVEL   LOG_INFO
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[mate_ble]: " __VA_ARGS__)
#define _LOGINF(...) LOG_INFO("[mate_ble]: " __VA_ARGS__)

/* include sound module only on SenseMate (gate has no audio) */
#if RIOT_CONFIG_DEVICE_TYPE == DEVICE_TYPE_SENSEMATE
#include "include/soundModule.h"
#include "events_creation.h"
#endif

event_t *_mateble_send_event = NULL;
event_queue_t *_mateble_send_queue = NULL;
event_timeout_t *_mateble_periodic_send_event_timeout;

static void mateble_send_event_handler(event_t *e);

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

typedef struct {
    uint8_t *buf;
    size_t buf_len;
    nimble_scanner_info_t info;
} payload_descriptor_t;

static kernel_pid_t _ble_receive_pid = KERNEL_PID_UNDEF;

/* max record / serialization sizes
 * TODO: obtain from records/tables interface ? */
#define MAX_SIGNATURE_SIZE 80
#define MAX_SERIALIZED_RECORD_SIZE 128

#define MATE_BLE_TX_POWER_UNDEF (127)

#define MATE_BLE_NIMBLE_INSTANCE (0)

/* The scan window defines how long to listen on the currently
 * configured advertisement channel. */
#define MATE_BLE_SCAN_WINDOW_MS 30

/* The scan interval defines how long to wait before starting the next
 * scan window an the next advertisement channel. Setting the
 * MATE_BLE_SCAN_INTERVAL_MS = MATE_BLE_SCAN_WINDOW_MS results in continuous scanning.*/
#define MATE_BLE_SCAN_INTERVAL_MS 30
#define MATE_BLE_ADV_START_MS 20
#define MATE_BLE_ADV_STOP_MS 200

#define MATE_BLE_THRESHOLD  (-85)

/* Message queue size for offloading received data
 * from the nimble callback to the rx thread context.
 * In the thread context heavier processing such as signature verification
 * and table traversal and merges can be done. */
#define RX_MSG_QUEUE_SIZE (8)

static uint8_t id_addr_type;
static uint8_t ble_initialized = 0;

/* Singleton reference to the tables instance. Is provided on init. */
static tables_context_t *_tables = NULL;

static const char adv_name[] = BLE_ADVERTISE_NAME;

/* The first two bytes of the manufacturer specific data type contain
 * a company ID code which for a final product must be requested from
 * the Bluetooth SIG. 
 * For testing purposes we use 0xFFFF as an unassigned identifier code.
 */
static const uint8_t _company_id_code[] = { 0xFF, 0xFF };

/* We use the manufacturer specific data type to transmit arbitrary data.
 * After the company ID we always put the same marker pattern to be able
 * to easily filter for these packets.
 */
static const uint8_t _custom_msd_marker_pattern[] = {
        0xf0, 0x9f, 0x93, 0x9f
};

/* payload offset within the custom marked  manufacturer specific data field */
#define MATE_BLE_MSD_PAYLOAD_OFFS (sizeof(_company_id_code) + \
                          sizeof(_custom_msd_marker_pattern))

/* TODO: This semaphore was previously used to synchronize to the advertisement done
 * event. An unresolved bug resulted in endless waiting for the semaphore to be posted.
 * It is currently worked around with a timer based delay but it should eventually be
 * replaced by a proper synchronization method.
 **/
//static sem_t adv_done_sem;

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg);
static void ad_append(bluetil_ad_t *ad, const uint8_t *data, unsigned len);
static void ad_append_marked_msd_payload(bluetil_ad_t *ad, const uint8_t *payload, unsigned len);
static void start_adv(uint8_t *payload, unsigned payload_len);
static void nimble_scan_evt_cb(uint8_t type, const ble_addr_t *addr, const nimble_scanner_info_t *info, const uint8_t *ad, size_t len);

static void ad_append(bluetil_ad_t *ad, const uint8_t *data, unsigned len)
{
    if ((ad->pos + len) <= ad->size) {
        memcpy(&ad->buf[ad->pos], data, len);
        ad->pos += len;
    }
}

/* define how to build manufacturer specific payload */
/* hand-craft a manufacturer specific data type with a custom marker at the start of the data */
static void ad_append_marked_msd_payload(bluetil_ad_t *ad, const uint8_t *payload, unsigned len)
{
    uint8_t msd_len = sizeof(_company_id_code) + 1 +
                      sizeof(_custom_msd_marker_pattern) + len;
    uint8_t data_type = BLE_GAP_AD_VENDOR;

    // set the size field
    ad_append(ad, &msd_len, sizeof(msd_len));
    // set the data type
    ad_append(ad, &data_type, sizeof(data_type));
    // set the company id code
    ad_append(ad, _company_id_code, sizeof(_company_id_code));
    // set the marker
    ad_append(ad, _custom_msd_marker_pattern, sizeof(_custom_msd_marker_pattern));
    // set the payload
    ad_append(ad, payload, len);
}

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg) 
{
    (void)arg;
    switch (event->type) {
        case BLE_GAP_EVENT_ADV_COMPLETE:
            //sem_post(&adv_done_sem);
            break;
        default:
            break;
    }
    return 0;
}

/* add function to configure advertisements with a custom payload */
static void start_adv(uint8_t *payload, unsigned payload_len)
{
    (void)payload;
    (void)payload_len;

    // Initialize data structures and configure advertisement parameters buffer for the advertisement
    static uint8_t adv_buf[MATE_BLE_ADV_PKT_BUFFER_SIZE];
    struct os_mbuf *data;
    int rc;
    struct ble_gap_ext_adv_params params;

    // advertising data struct
    static bluetil_ad_t ad;

    // use defaults for non-set params
    memset(&params, 0, sizeof(params));

    /* advertise using ID addr */
    params.own_addr_type = id_addr_type;

    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_1M;
    params.tx_power = MATE_BLE_TX_POWER_UNDEF;
    params.sid = 0;
    // min/max advertising interval converted from ms to 0.625ms units
    params.itvl_min = BLE_GAP_ADV_ITVL_MS(MATE_BLE_ADV_START_MS);
    params.itvl_max = BLE_GAP_ADV_ITVL_MS(MATE_BLE_ADV_STOP_MS);

    // configure the nimble instance
    rc = ble_gap_ext_adv_configure(MATE_BLE_NIMBLE_INSTANCE, &params, NULL, ble_gap_event_cb, NULL);
    assert (rc == 0);

    // Create a new advertisement packet get mbuf for adv data
    data = os_msys_get_pkthdr(MATE_BLE_ADV_PKT_BUFFER_SIZE, 0);
    assert(data);

    /* build advertising data with flags to specifiy that:
    * - the device is a BLE device (instead of BR/EDR a.k.a. bluetooth classic)
    * - the device is not discoverable */
    rc = bluetil_ad_init_with_flags(&ad, adv_buf, sizeof(adv_buf),
            BLE_GAP_FLAG_BREDR_NOTSUP);
    assert(rc == BLUETIL_AD_OK);

    // give the device a name that is included in the advertisements
    rc = bluetil_ad_add_name(&ad, adv_name);
    assert(rc == BLUETIL_AD_OK);

    // Append a manufacturer specific data type. Add a manufacturer specific data entry with custom marker.
    ad_append_marked_msd_payload(&ad, payload, payload_len);

    // fill mbuf with adv data
    rc = os_mbuf_append(data, ad.buf, ad.pos);
    assert(rc == 0);

    rc = ble_gap_ext_adv_set_data(MATE_BLE_NIMBLE_INSTANCE, data);
    assert (rc == 0);

    _LOGDBG("triggering advertisement...\n");
    if (LOG_LEVEL >= LOG_DEBUG) {
        od_hex_dump(payload, payload_len, 0);
    }
    //tables_print_all();

    // Start advertising
    rc = ble_gap_ext_adv_start(MATE_BLE_NIMBLE_INSTANCE, 0, 1);
    assert (rc == 0);
}

static void nimble_scan_evt_cb(uint8_t type, const ble_addr_t *addr,
                        const nimble_scanner_info_t *info,
                        const uint8_t *ad, size_t len)
{
    assert(addr);
    (void)type;
    (void)ad;
    (void)len;

    // ignore legacy advertisements
    if (!(type & NIMBLE_SCANNER_EXT_ADV)) {
        return;
    }

    // parse the name of advertised devices
    bluetil_ad_t rec_ad;

    // drop const of ad with cast. Ensure read-only access!
    uint8_t *ad_ro = (uint8_t*)ad;
    bluetil_ad_init(&rec_ad, ad_ro, len, len);

    char name[BLE_ADV_PDU_LEN + 1] = {0};
    int table_result = bluetil_ad_find_str(&rec_ad, BLE_GAP_AD_NAME,
                                name, sizeof(name));
    // Output name, address, and data of the advertisement
    if (table_result == BLUETIL_AD_OK) {
        _LOGDBG("\"%s\" @", name);
    }
    if (LOG_LEVEL >= LOG_DEBUG) {
        nimble_addr_print(addr);
    }
    _LOGDBG("sent %d bytes\n", len);
    if (LOG_LEVEL >= LOG_DEBUG) {
        od_hex_dump(ad, len, 0);
    }

    // output our payload marke# BUILD_IN_DOCKER ?= 1d by our custom byte pattern
    bluetil_ad_data_t msd;
    table_result = bluetil_ad_find(&rec_ad, BLE_GAP_AD_VENDOR, &msd);
    if (table_result == BLUETIL_AD_OK) {
        uint8_t *marker = &msd.data[sizeof(_company_id_code)];
        if (memcmp(marker, _custom_msd_marker_pattern,
                sizeof(_custom_msd_marker_pattern)) == 0) {
            uint8_t *payload = &msd.data[MATE_BLE_MSD_PAYLOAD_OFFS];

            // length of the payload without the marker
            int pl = msd.len - MATE_BLE_MSD_PAYLOAD_OFFS;

            payload_descriptor_t *pd = malloc(sizeof(payload_descriptor_t));
            if (pd) {
                uint8_t *buf = malloc(pl);
                if (buf) {
                    memcpy(buf, payload, pl);
                    pd->buf = buf;
                    pd->buf_len = pl;
                    pd->info = *info;
                    msg_t offload_msg = { .content.ptr = pd };
                    if (msg_send(&offload_msg, _ble_receive_pid) != 1) {
                        _LOGDBG("offload to RX thread failed -> discard packet!\n");
                        free(buf);
                        free(pd);
                    } else {
                        _LOGDBG("offloaded %d bytes to RX thread\n", pl);
                    }
                } else {
                    _LOGDBG("malloc of payload buffer failed (%d B) -> discard packet!\n", pl);
                    free(pd);
                }
            } else {
                _LOGDBG("malloc payload descriptor failed! -> discard packet!\n");
            }
        }
    }
}

int mate_ble_init(tables_context_t *tables)
{
    _LOGDBG("Initializing BLE extended advertisement...\n");
    _tables = tables;

    //sem_init(&adv_done_sem, 0, 0);

    // Make sure we have proper identity address set (public preferred)
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    // configure global address
    rc = ble_hs_id_infer_auto(0, &id_addr_type);
    assert(rc == 0);

    // Initialize the nimble scanner
    nimble_scanner_cfg_t params = {
        .itvl_ms = MATE_BLE_SCAN_INTERVAL_MS,
        .win_ms = MATE_BLE_SCAN_WINDOW_MS,
        .flags = NIMBLE_SCANNER_PHY_1M,
    };
    // initialize the scanner and set up our own callback
    nimble_scanner_init(&params, nimble_scan_evt_cb);
    // start the scanner
    nimble_scanner_start();
    ble_initialized = 1;
    return BLE_SUCCESS;
}

static int _ble_send(uint8_t *buf, size_t len)
{
    /* if advertising is already active stop it before updating
    * the advertised content */
    if (ble_gap_ext_adv_active(MATE_BLE_NIMBLE_INSTANCE)) {
        ble_gap_ext_adv_stop(MATE_BLE_NIMBLE_INSTANCE);
    }

    // update the payload with the given message
    start_adv(buf, len);

    // Block here until the ADV_COMPLETE event posts the sem
    //sem_wait(&adv_done_sem);
    ztimer_sleep(ZTIMER_MSEC, MATE_BLE_ADV_STOP_MS);

    _LOGDBG("stopping advertisement...\n");
    ble_gap_ext_adv_stop(MATE_BLE_NIMBLE_INSTANCE);
    _LOGDBG("stopped advertisement...\n");

    return BLE_SUCCESS;
}

static void wait_for_ble_init(void)
{
    if (!ble_initialized) {
        LOG_DEBUG("ble_send_loop: BLE not initialized\n");
        while (!ble_initialized) {
            ztimer_sleep(ZTIMER_MSEC, 100);
        }
    }
}
char _send_record_str_buf[TABLE_RECORD_STRING_SIZE];

static int _send_record(const table_record_t *record)
{
    size_t out_len = MAX_SERIALIZED_RECORD_SIZE;
    uint8_t out_buf[out_len];

    int res = cbor_serialize_record(record, out_buf, &out_len);
    _LOGDBG("%s serialize:(%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        _LOGINF("_send_record serialize failed!\n");
        return -1;
    }

    record_tostr(record, _send_record_str_buf,
            sizeof(_send_record_str_buf));
    _LOGINF("TX %s\n", _send_record_str_buf);

    if (LOG_LEVEL == LOG_DEBUG) {
        _LOGDBG("_send_record: \n");
        od_hex_dump(out_buf, out_len, 0);
    }
    return _ble_send(out_buf, out_len);
}

static void mateble_send_event_handler(event_t *e)
{
    (void)e;
    _LOGDBG("%s\n", __func__);
    TABLE_ITERATOR(iterator, _tables);

    table_record_t *record;
    table_query_t query = {
        //TODO: ignore non-propageted records
        .type = RECORD_UNDEFINED,
        .writer_id = NULL,
        .involved_id = NULL
    };

    /* fixed worst-case signature buffer size to avoid multiple iterator calls */
    uint8_t signature[MAX_SIGNATURE_SIZE];
    size_t signature_len = sizeof(signature);

    /* TODO: check if the other pattern with multiple calls to the iterator is safe to do.
     *     First getting the size, then the record seems problematic if the iterator may change
     *     inbetween.
     *     Maybe, a separate api to get the signature for a specific record would be better?
     */
    int res = tables_iterator_init(_tables, &iterator, &query);
    _LOGDBG("%s iter init (%d) %s\n", __func__, res, ok(res == 0));
    if (res) {
        event_timeout_set(_mateble_periodic_send_event_timeout, BLE_SEND_INTERVAL);
        return;
    }

    while(tables_iterator_next(_tables, &iterator, &record, signature, &signature_len) == 0) {
        //_LOGDBG("%s iter next (%d) %s\n", __func__, res, ok(res == 0));
        res = _send_record(record);
        _LOGDBG("%s _send_record: %d\n", __func__, res);
    }
    event_timeout_set(_mateble_periodic_send_event_timeout, BLE_SEND_INTERVAL);
}

void* ble_send_loop(void* arg)
{
    //TODO: properly integrate the thread args to pass event queues,
    //      events etc. for signalling stuff to other components.
    //ble_tx_thread_args_t *thr_args = (ble_tx_thread_args_t*)arg;
    (void)arg;

    _LOGDBG("%s\n", __func__);

    wait_for_ble_init();

    // TODO: debug issue with heap allocated events getting corrupted
    event_t mateble_send_event = { .handler = mateble_send_event_handler,
                               .list_node.next = NULL };

    event_queue_t mateble_send_queue;

    event_timeout_t mateble_periodic_send_event_timeout;
    _mateble_periodic_send_event_timeout = &mateble_periodic_send_event_timeout;

    event_queue_init(&mateble_send_queue);

    event_timeout_ztimer_init(&mateble_periodic_send_event_timeout, ZTIMER_MSEC,
                              &mateble_send_queue,
                              &mateble_send_event);

    event_timeout_set(&mateble_periodic_send_event_timeout, BLE_SEND_INTERVAL);

    _mateble_send_queue = &mateble_send_queue;
    _mateble_send_event = &mateble_send_event;

    event_loop(&mateble_send_queue);

    return NULL;
}

char _recv_record_str_buf[TABLE_RECORD_STRING_SIZE];

void* ble_receive_loop(void* args)
{
    _ble_receive_pid = thread_getpid();

    (void)args;
    //TODO: same as for the send loop: integrate the parameters
    //ble_receive_thread_args_ptr_t thr_args = (ble_receive_thread_args_ptr_t)args;
    
    static msg_t rx_msg_queue[RX_MSG_QUEUE_SIZE];

    /* initialize the message queue] */
    msg_init_queue(rx_msg_queue, RX_MSG_QUEUE_SIZE);

    wait_for_ble_init();

    while (1) {
        msg_t msg;
        _LOGDBG("ble_receive_loop waiting for msg...\n");
        msg_receive(&msg);
        payload_descriptor_t *pd = (payload_descriptor_t*)msg.content.ptr;
        //TODO interpret pd->info to decide if we ignore low RSSI packet;

        table_record_t record;
        table_record_data_buffer_t record_data;
        uint8_t signature[MAX_SIGNATURE_SIZE];
        size_t signature_len = sizeof(signature);

        _LOGDBG("received %d bytes (RSSI %d):\n", pd->buf_len, pd->info.rssi);
        if (LOG_LEVEL >= LOG_DEBUG) {
            od_hex_dump(pd->buf, pd->buf_len, 0);
        }

        int res = cbor_deserialize(pd->buf, pd->buf_len, &record,
                                    &record_data, signature, &signature_len);

        free(pd->buf);
        free(pd);
        _LOGDBG("freed buffers\n");

        if (res) {
            _LOGDBG("cbor_deserialize failed: %d\n", res);
            continue;
        }

        _LOGDBG("signature length: %d\n", signature_len);

        record_tostr(&record, _recv_record_str_buf,
                     sizeof(_recv_record_str_buf));
        _LOGINF("RX %s\n", _recv_record_str_buf);

        _LOGDBG("trying to merge record...\n");
        table_merge_result_t result;
        res = tables_merge_record(_tables, &record, &result);
        if (res) {
            _LOGDBG("tables_merge_record failed: %d\n", res);
        }

        if (!res && (result.updated || result.new)) {
#if RIOT_CONFIG_DEVICE_TYPE == DEVICE_TYPE_SENSEMATE
            event_post(EVENT_PRIO_MEDIUM, &eventBleNews);
            _LOGINF("table updated.\n");
#endif
        } else if (result.rejected_sig || result.invalid_record){
            _LOGINF("Error updating table.\n");
        } else {
#if RIOT_CONFIG_DEVICE_TYPE == DEVICE_TYPE_SENSEMATE
            event_post(EVENT_PRIO_MEDIUM, &eventBleRx);
#endif
            _LOGINF("No updates.\n");
        }
    }
    /* never reached */
    return NULL;
}
