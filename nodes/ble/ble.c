#include "ble.h"

#include <stdio.h>
#include <stdlib.h>
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "nimble_scanner.h"
#include "nimble_addr.h"
#include "nimble/ble.h"
#include "net/bluetil/ad.h"
#include "shell.h"
#include "timex.h"
#include "ztimer.h"

#define BLE_TX_POWER_UNDEF (127)

#define BLE_GAP_NAME_BUF_SIZE (300)
#define BLE_ADV_PKT_BUFFER_SIZE (300)
#define BLE_NIMBLE_INSTANCE (0)

/* The scan window defines how long to listen on the currently
 * configured advertisement channel. */
#define BLE_SCAN_WINDOW_MS    30

/* The scan interval defines how long to wait before starting the next
 * scan window an the next advertisement channel. Setting the
 * BLE_SCAN_INTERVAL_MS = BLE_SCAN_WINDOW_MS results in continuous scanning.*/
#define BLE_SCAN_INTERVAL_MS    30

static uint8_t id_addr_type;

static const char adv_name[] = "RIOT-ext-adv";

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
#define BLE_MSD_PAYLOAD_OFFS (sizeof(_company_id_code) + \
                          sizeof(_custom_msd_marker_pattern))

/* buffer to store the advertised data in */
static uint8_t _payload_buf[BLE_ADV_PKT_BUFFER_SIZE];
static unsigned _pl_len = 0;

void _ad_append(bluetil_ad_t *ad, const uint8_t *data, unsigned len)
{
    if ((ad->pos + len) <= ad->size) {
        memcpy(&ad->buf[ad->pos], data, len);
        ad->pos += len;
    }
}

/* [TASK 2.6: define how to build manufacturer specific payload] */
/* hand-craft a manufacturer specific data type with a custom marker at the start of the data */
static void _ad_append_marked_msd_payload(bluetil_ad_t *ad, const uint8_t *payload, unsigned len)
{
    uint8_t msd_len = sizeof(_company_id_code) + 1 +
                      sizeof(_custom_msd_marker_pattern) + len;
    uint8_t data_type = BLE_GAP_AD_VENDOR;

    /* set the size field */
    _ad_append(ad, &msd_len, sizeof(msd_len));

    /* set the data type */
    _ad_append(ad, &data_type, sizeof(data_type));

    /* set the company id code */
    _ad_append(ad, _company_id_code, sizeof(_company_id_code));

    /* set the marker */
    _ad_append(ad, _custom_msd_marker_pattern, sizeof(_custom_msd_marker_pattern));

    /* set the payload */
    _ad_append(ad, payload, len);
}

/* [TASK 2.1: add function to configure advertisements with a custom payload] */
void start_adv(uint8_t *payload, unsigned payload_len)
{
    (void)payload;
    (void)payload_len;

    /* [TASK 2.2: Initialize data structures and configure advertisement parameters] */
    /* buffer for the advertisement */
    static uint8_t adv_buf[ABLE_DV_PKT_BUFFER_SIZE];
    struct os_mbuf *data;
    int rc;
    struct ble_gap_ext_adv_params params;

    /* advertising data struct */
    static bluetil_ad_t ad;

    /* use defaults for non-set params */
    memset (&params, 0, sizeof(params));

    /* advertise using ID addr */
    params.own_addr_type = id_addr_type;

    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_1M;
    params.tx_power = BLE_TX_POWER_UNDEF;
    params.sid = 0;
    /* min/max advertising interval converted from ms to 0.625ms units */
    params.itvl_min = BLE_GAP_ADV_ITVL_MS(600);
    params.itvl_max = BLE_GAP_ADV_ITVL_MS(800);

    /* configure the nimble instance */
    rc = ble_gap_ext_adv_configure(BLE_NIMBLE_INSTANCE, &params, NULL, NULL, NULL);
    assert (rc == 0);

    /* [TASK 2.3: Create a new advertisement packet] */
    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(BLE_ADV_PKT_BUFFER_SIZE, 0);
    assert(data);

    /* build advertising data with flags to specifiy that:
    * - the device is a BLE device (instead of BR/EDR a.k.a. bluetooth classic)
    * - the device is not discoverable */
    rc = bluetil_ad_init_with_flags(&ad, adv_buf, sizeof(adv_buf),
            BLE_GAP_FLAG_BREDR_NOTSUP);
    assert(rc == BLUETIL_AD_OK);

    /* give the device a name that is included in the advertisements */
    rc = bluetil_ad_add_name(&ad, adv_name);
    assert(rc == BLUETIL_AD_OK);

    /* [TASK 2.4: Append a manufacturer specific data type] */
    /* Add a manufacturer specific data entry with custom marker. */
    _ad_append_marked_msd_payload(&ad, payload, payload_len);

    /* fill mbuf with adv data */
    rc = os_mbuf_append(data, ad.buf, ad.pos);
    assert(rc == 0);

    rc = ble_gap_ext_adv_set_data(BLE_NIMBLE_INSTANCE, data);
    assert (rc == 0);

    /* [TASK 2.5: Start advertising] */
    rc = ble_gap_ext_adv_start(BLE_NIMBLE_INSTANCE, 0, 0);
    assert (rc == 0);

    printf("Now advertising \"%s\"\n", payload);
}

/* [TASK 2.7: Add another shell comand for changing the advertised data] */
int cmd_adv(int argc, char **argv)
{
    /* check that the command is called correctly */
    if (argc != 2) {
        puts("usage: adv <message>");
        return 1;
    }

    /* if advertising is already active stop it before updating
     * the advertised content */
    if (ble_gap_ext_adv_active(BLE_NIMBLE_INSTANCE)) {
        ble_gap_ext_adv_stop(BLE_NIMBLE_INSTANCE);
    }

    _pl_len = strlen(argv[1]);

    /* update the payload with the given message */
    memcpy(_payload_buf, argv[1], _pl_len);

    start_adv(_payload_buf, _pl_len);

    return 0;
}
SHELL_COMMAND(adv,"set advertised message",cmd_adv);

void _print_hex_arr(const uint8_t *data, unsigned len)
{
    printf("{");
    for (unsigned i = 0; i < len; i++) {
        printf(" 0x%02x%c", data[i], (i == len-1) ? ' ' : ',');
    }
    printf("}\n");
}

/* [TASK 1.2: Implement a custom callback for scan events] */
void nimble_scan_evt_cb(uint8_t type, const ble_addr_t *addr,
                        const nimble_scanner_info_t *info,
                        const uint8_t *ad, size_t len)
{
    assert(addr);
    (void)type;
    (void)info;
    (void)ad;
    (void)len;

    /* [TASK 3.1: filter for extended advertisements only] */
    /* ignore legacy advertisements */
    if (!(type & NIMBLE_SCANNER_EXT_ADV)) {
        return;
    }

    /* [TASK 1.3: parse the name of advertised devices] */
    bluetil_ad_t rec_ad;

    /* drop const of ad with cast. Ensure read-only access! */
    uint8_t *ad_ro = (uint8_t*)ad;
    bluetil_ad_init(&rec_ad, ad_ro, len, len);

    char name[BLE_ADV_PDU_LEN + 1] = {0};
    int res = bluetil_ad_find_str(&rec_ad, BLE_GAP_AD_NAME,
                                name, sizeof(name));
    /* [TASK 1.4: Output name, address, and data of the advertisement] */
    if (res == BLUETIL_AD_OK) {
        printf("\n\"%s\" @", name);
    }
    nimble_addr_print(addr);
    printf("sent %d bytes:\n", len);
    _print_hex_arr(ad, len);

    /* [TASK 3.2: output our payload marked by our custom byte pattern] */
    bluetil_ad_data_t msd;
    res = bluetil_ad_find(&rec_ad, BLE_GAP_AD_VENDOR, &msd);
    if (res == BLUETIL_AD_OK) {
        uint8_t *marker = &msd.data[sizeof(_company_id_code)];
        if (memcmp(marker, _custom_msd_marker_pattern,
                sizeof(_custom_msd_marker_pattern)) == 0) {
            uint8_t *payload = &msd.data[BLE_MSD_PAYLOAD_OFFS];
            /* length of the payload without the marker */
            int pl = msd.len - BLE_MSD_PAYLOAD_OFFS;
            printf("%.*s\n", pl, payload);
        }
    }
}

/* [TASK 1.5: add a shell command to start and stop scanning] */
int cmd_scan(int argc, char **argv)
{
    if (argc == 2) {
        if (strcmp("start", argv[1]) == 0) {
            nimble_scanner_start();
            return 0;
        } else if (strcmp("stop", argv[1]) == 0) {
            nimble_scanner_stop();
            return 0;
        }
    }

    printf("usage: %s start|stop\n", argv[0]);
    return 0;
}
SHELL_COMMAND(scan,"start/stop BLE scanning",cmd_scan);


void ble_example()
{
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("BLE extended advertisement app!");

    int rc;
    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    /* configure global address */
    rc = ble_hs_id_infer_auto(0, &id_addr_type);
    assert(rc == 0);

    /* [TASK 1.1: initialize the nimble scanner ] */
    /* [TASK 1.1: initialize the nimble scanner ] */
    nimble_scanner_cfg_t params = {
        .itvl_ms = BLE_SCAN_INTERVAL_MS,
        .win_ms = BLE_SCAN_WINDOW_MS,
        .flags = BLE_NIMBLE_SCANNER_PHY_1M,
    };

    /* initialize the scanner and set up our own callback */
    nimble_scanner_init(&params, nimble_scan_evt_cb);

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return;
}
/** @} */