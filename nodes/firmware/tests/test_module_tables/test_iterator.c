#include "time.h"
#include "store_service.h"
#include "crypto_service.h"
#include "hybrid_logical_clock.h"
#include "periph/rtc.h"

#include "store.h"
#include "crypto.h"
#include "hlc_ztimer.h"

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

static void _setup_test(tables_context_t *ctx,
                        store_ctx *store_context,
                        store_service_t *store_service,
                        crypto_ctx_t *crypto_context,
                        crypto_service_t *crypto_service,
                        hlc_ctx_t *hlc_ctx,
                        hlc_ztimer_t *hlc_ztimer_ctx,
                        const node_id_t *self_id)
{
    store_service->interface = store_interface;
    store_service->context = store_context;
    store_init_ctx(store_context);

    crypto_service->interface = crypto_interface;
    crypto_service->context = crypto_context;
    crypto_init_ctx(crypto_context, (const uint8_t[]){ DUMMY_SIGNATURE },
                    DUMMY_SIGNATURE_LEN);

    TEST_ASSERT_EQUAL_INT(0, hlc_ztimer_init(hlc_ztimer_ctx, 0));

    TEST_ASSERT_EQUAL_INT(0, hlc_init(hlc_ctx, hlc_ztimer_get_time, hlc_ztimer_ctx));

    TEST_ASSERT_EQUAL_INT(0,
        tables_init(ctx, self_id, store_service, crypto_service, hlc_ctx)
    );
}

static void test_tables_iterator(void)
{
    tables_context_t ctx;
    node_id_t self_id = { GATE_TEST_ID };
    node_id_t mate_id = { MATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    hlc_ztimer_t hlc_ztimer_ctx;
    store_ctx store_context;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &hlc_ztimer_ctx, &self_id);

    /* Add some records */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_OPEN));
    TEST_ASSERT_EQUAL_INT(0, tables_put_mate_encounter(&ctx, &mate_id, 0));

    TABLE_ITERATOR(iterator, &ctx);

    table_query_t query = {
        .type = RECORD_GATE_REPORT,
        .writer_id = &self_id
    };

    TEST_ASSERT_EQUAL_INT(0, tables_iterator_init(&ctx, &iterator, &query));

    table_record_t *record;
    const node_id_t *id;
    table_record_type_t type;
    TEST_ASSERT_EQUAL_INT(0, tables_iterator_next(&ctx, &iterator, &record, NULL, NULL));

    get_record_type(record, &type);
    TEST_ASSERT_EQUAL_INT(RECORD_GATE_REPORT, type);

    get_record_writer_id(record, &id);
    TEST_ASSERT_EQUAL_MEMORY(self_id, *id, NODE_ID_SIZE);

    gate_state_t state;
    table_gate_report_t *report;
    TEST_ASSERT_EQUAL_INT(0, get_gate_report_data(record, &report));
    get_gate_report_state(report, &state);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_OPEN, state);

    // there should be no more matching records
    TEST_ASSERT_EQUAL_INT(-1, tables_iterator_next(&ctx, &iterator, &record, NULL, NULL));

    query.type = RECORD_UNDEFINED;
    query.involved_id = &mate_id;
    query.writer_id = NULL;

    TEST_ASSERT_EQUAL_INT(0, tables_iterator_init(&ctx, &iterator, &query));

    TEST_ASSERT_EQUAL_INT(0, tables_iterator_next(&ctx, &iterator, &record, NULL, NULL));

    get_record_type(record, &type);
    TEST_ASSERT_EQUAL_INT(RECORD_MATE_ENCOUNTER, type);

    get_record_writer_id(record, &id);
    TEST_ASSERT_EQUAL_MEMORY(self_id, *id, NODE_ID_SIZE);

    // there should be no more matching records
    TEST_ASSERT_EQUAL_INT(-1, tables_iterator_next(&ctx, &iterator, &record, NULL, NULL));
}


void test_iterator(void)
{
    RUN_TEST(test_tables_iterator);
}
