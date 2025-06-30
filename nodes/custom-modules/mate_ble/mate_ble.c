#include "mate_ble.h"

#include <stdio.h>
#include <stdlib.h>
#include <kernel_sem.h>

#include "host/util/util.h"
#include "host/ble_hs.h"
#include "nimble_scanner.h"
#include "nimble_addr.h"
#include "nimble/ble.h"
#include "net/bluetil/ad.h"
#include "shell.h"
#include "timex.h"
#include "ztimer.h"
#include "cbor.h"

#include "incoming_list.h"

#define MATE_BLE_TX_POWER_UNDEF (127)

#define MATE_BLE_GAP_NAME_BUF_SIZE (300)
#define MATE_BLE_ADV_PKT_BUFFER_SIZE BLE_MAX_PAYLOAD_SIZE
#define MATE_BLE_NIMBLE_INSTANCE (0)

/* The scan window defines how long to listen on the currently
 * configured advertisement channel. */
#define MATE_BLE_SCAN_WINDOW_MS    30

/* The scan interval defines how long to wait before starting the next
 * scan window an the next advertisement channel. Setting the
 * MATE_BLE_SCAN_INTERVAL_MS = MATE_BLE_SCAN_WINDOW_MS results in continuous scanning.*/
#define MATE_BLE_SCAN_INTERVAL_MS    30

static uint8_t id_addr_type;

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

/* buffer to store the advertised data in */
static uint8_t _payload_buf[MATE_BLE_ADV_PKT_BUFFER_SIZE];
//static unsigned _pl_len = 0;

static sem_t adv_done_sem;

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg);
static void ad_append(bluetil_ad_t *ad, const uint8_t *data, unsigned len);
static void ad_append_marked_msd_payload(bluetil_ad_t *ad, const uint8_t *payload, unsigned len);
static void start_adv(uint8_t *payload, unsigned payload_len);
static void print_hex_arr(const uint8_t *data, unsigned len);
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
switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
        puts("Advertisement completed.\n");
        sem_post(&adv_done_sem);
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
    params.itvl_min = BLE_GAP_ADV_ITVL_MS(20);
    params.itvl_max = BLE_GAP_ADV_ITVL_MS(20);

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

    // Start advertising
    rc = ble_gap_ext_adv_start(MATE_BLE_NIMBLE_INSTANCE, 0, 1);
    assert (rc == 0);

    printf("Now advertising \"%s\"\n", payload);
}

static void print_hex_arr(const uint8_t *data, unsigned len)
{
    printf("{");
    for (unsigned i = 0; i < len; i++) {
        printf(" 0x%02x%c", data[i], (i == len-1) ? ' ' : ',');
    }
    printf("}\n");
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
    int res = bluetil_ad_find_str(&rec_ad, BLE_GAP_AD_NAME,
                                name, sizeof(name));
    // Output name, address, and data of the advertisement
    if (res == BLUETIL_AD_OK) {
        printf("\n\"%s\" @", name);
    }
    nimble_addr_print(addr);
    printf("sent %d bytes:\n", len);
    print_hex_arr(ad, len);

    // output our payload marked by our custom byte pattern
    bluetil_ad_data_t msd;
    res = bluetil_ad_find(&rec_ad, BLE_GAP_AD_VENDOR, &msd);
    if (res == BLUETIL_AD_OK) {
        uint8_t *marker = &msd.data[sizeof(_company_id_code)];
        if (memcmp(marker, _custom_msd_marker_pattern,
                sizeof(_custom_msd_marker_pattern)) == 0) {
            uint8_t *payload = &msd.data[MATE_BLE_MSD_PAYLOAD_OFFS];
            // length of the payload without the marker
            int pl = msd.len - MATE_BLE_MSD_PAYLOAD_OFFS;
            printf("Received: %.*s\n", pl, payload);

            ble_metadata_t metadata = {};
            metadata.message_type = 187;
            metadata.rssi = info->rssi;
            insert_message(payload, pl, metadata);
        }
    }
}

int ble_init(void)
{
    puts("Initializing BLE extended advertisement!");

    sem_init(&adv_done_sem, 0, 0);

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

    return BLE_SUCCESS;
}

int ble_receive(cbor_message_type_t type, cbor_buffer* cbor_packet, ble_metadata_t* metadata)
{
    return wait_for_message(type, cbor_packet, metadata);
}

int ble_send(cbor_buffer* cbor_packet)
{
    /* if advertising is already active stop it before updating
    * the advertised content */
    if (ble_gap_ext_adv_active(MATE_BLE_NIMBLE_INSTANCE)) {
        ble_gap_ext_adv_stop(MATE_BLE_NIMBLE_INSTANCE);
    }

    // update the payload with the given message
    memcpy(_payload_buf, cbor_packet->buffer, cbor_packet->cbor_size);
    start_adv(_payload_buf, cbor_packet->cbor_size);

    /* Block here until the ADV_COMPLETE event posts the sem */
    sem_wait(&adv_done_sem);

    ble_gap_ext_adv_stop(MATE_BLE_NIMBLE_INSTANCE);

    return BLE_SUCCESS;
}

void ble_send_loop(void)
{
    if (ble_init() != BLE_SUCCESS) {
        return;
    }

    uint8_t stack_buffer[BLE_MAX_PAYLOAD_SIZE];
    cbor_buffer buffer;
    buffer.buffer = stack_buffer;
    buffer.capacity = BLE_MAX_PAYLOAD_SIZE;
    ble_metadata metadata;

    while (true) {
        int count = target_state_table_to_cbor_many(BLE_MAX_PAYLOAD_SIZE, &buffer);
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                ble_send(&buffer);
            }
        }

        count = is_state_table_to_cbor_many(BLE_MAX_PAYLOAD_SIZE, &buffer);
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                ble_send(&buffer);
            }
        }
        
        int count = seen_status_table_to_cbor_many(BLE_MAX_PAYLOAD_SIZE, &buffer);
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                ble_send(&buffer);
            }
        }

        count = jobs_table_to_cbor_many(BLE_MAX_PAYLOAD_SIZE, &buffer);
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                ble_send(&buffer);
            }
        }
    }
}