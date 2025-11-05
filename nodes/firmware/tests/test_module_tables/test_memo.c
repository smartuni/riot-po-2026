#include "time.h"
#include "store_service.h"
#include "crypto_service.h"
#include "hybrid_logical_clock.h"
#include "periph/rtc.h"

#include "store.h"
#include "crypto.h"
#include "hlc_rtc.h"

#include "tables.h"
#include "tables/types.h"
#include "tables/records.h"

#define GATE_TEST_ID 0x01, 0x02, 0x03, 0x04
#define MATE_TEST_ID 0x05, 0x06, 0x07, 0x08
#define MATE_2_TEST_ID 0x09, 0x0A, 0x0B, 0x0C
#define DUMMY_SIGNATURE 0xAA, 0xBB, 0xCC, 0xDD, \
                        0xEE, 0xFF, 0x11, 0x22, \
                        0x33, 0x44, 0x55, 0x66

#define DUMMY_SIGNATURE_LEN 12

static table_record_t _record;
static table_query_t _query;
static int _arg;
static int _cb_calls = 0;

static void _setup_test(tables_context_t *ctx,
                        store_ctx *store_context,
                        store_service_t *store_service,
                        crypto_ctx_t *crypto_context,
                        crypto_service_t *crypto_service,
                        hlc_ctx_t *hlc_ctx,
                        const node_id_t *self_id)
{
    store_service->interface = store_interface;
    store_service->context = store_context;
    store_init_ctx(store_context);

    crypto_service->interface = crypto_interface;
    crypto_service->context = crypto_context;
    crypto_init_ctx(crypto_context, (const uint8_t[]){ DUMMY_SIGNATURE },
                    DUMMY_SIGNATURE_LEN);

    TEST_ASSERT_EQUAL_INT(0, hlc_init(hlc_ctx, hlc_rtc_get_time));

    TEST_ASSERT_EQUAL_INT(0,
        tables_init(ctx, self_id, store_service, crypto_service, hlc_ctx)
    );
}

static void _memo_cb(tables_context_t *ctx, const table_record_t *record,
                     const table_query_t *query, void *arg)
{
    TEST_ASSERT_NOT_NULL(ctx);
    TEST_ASSERT_NOT_NULL(record);
    TEST_ASSERT_NOT_NULL(query);
    TEST_ASSERT_NOT_NULL(arg);

    memcpy(&_record, record, sizeof(table_record_t));
    memcpy(&_query, query, sizeof(table_query_t));
    memcpy(&_arg, &arg, sizeof(int));

    _cb_calls++;
}

static void test_table_memo_on_write(void)
{
    tables_context_t ctx;
    node_id_t self_id = { GATE_TEST_ID };
    node_id_t mate_id = { MATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    store_ctx store_context;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_service_t store_service;
    const void *arg = (void *)0x55;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &self_id);

    table_memo_t memo;
    table_query_t query;

    tables_init_query(&query, RECORD_GATE_REPORT, NULL, NULL);
    tables_add_memo(&ctx, &memo, &query, _memo_cb, (void *)arg);

    /* Add record */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_OPEN));

    /* Our callback should've been called */
    TEST_ASSERT_EQUAL_INT(1, _cb_calls);
    TEST_ASSERT_EQUAL_INT(arg, _arg);
    TEST_ASSERT_EQUAL_INT(RECORD_GATE_REPORT, _record.header.type);

    /* Add a gate observation type */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_observation(&ctx, &self_id, GATE_STATE_OPEN));

    /* Our callback should not have been called */
    TEST_ASSERT_EQUAL_INT(1, _cb_calls);

    /* Remove the memo and change the query */
    TEST_ASSERT_EQUAL_INT(0, tables_remove_memo(&ctx, &memo));
    tables_init_query(&query, RECORD_UNDEFINED, NULL, &mate_id);
    tables_add_memo(&ctx, &memo, &query, _memo_cb, (void *)arg);

    /* Add record not involving the mate ID */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_CLOSED));

    /* Our callback should not have been called */
    TEST_ASSERT_EQUAL_INT(1, _cb_calls);

    /* Add a record involving the mate ID */
    TEST_ASSERT_EQUAL_INT(0, tables_put_mate_encounter(&ctx, &mate_id, 0));

    /* Our callback should've been called */
    TEST_ASSERT_EQUAL_INT(2, _cb_calls);
}

void test_memo(void)
{
    RUN_TEST(test_table_memo_on_write);
}
