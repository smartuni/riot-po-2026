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


#include "od.h"
static void test_tables_merge(void)
{
    tables_context_t ctx;
    node_id_t self_id = { GATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    store_ctx store_context;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &self_id);

    /* Add a record */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_OPEN));

    /* Get the record back */
    TABLE_ITERATOR(iterator, &ctx);

    table_record_t *record;
    table_query_t query = {
        .type = RECORD_GATE_REPORT,
        .writer_id = &self_id
    };

    // Get maximum signature required size
    TEST_ASSERT_EQUAL_INT(0, tables_iterator_init(&ctx, &iterator, &query));
    TEST_ASSERT_EQUAL_INT(0,
        tables_iterator_next(&ctx, &iterator, &record, NULL, NULL)
    );

    size_t signature_len = record->signature_len;
    uint8_t signature[signature_len];

    TEST_ASSERT_EQUAL_INT(0, tables_iterator_init(&ctx, &iterator, &query));
    TEST_ASSERT_EQUAL_INT(0,
        tables_iterator_next(&ctx, &iterator, &record, signature, &signature_len)
    );

    set_record_sequence(record, 3);
    table_merge_result_t result;

    // fake crypto will accept the signature even with changed record
    TEST_ASSERT_EQUAL_INT(0, tables_merge_record(&ctx, record, &result));
    TEST_ASSERT_TRUE(result.updated);

    set_record_sequence(record, 1);
    TEST_ASSERT_EQUAL_INT(-1, tables_merge_record(&ctx, record, &result));

    record->signature = NULL;
    TEST_ASSERT_EQUAL_INT(-1, tables_merge_record(&ctx, record, &result));
    TEST_ASSERT_TRUE(result.rejected_sig);
}


void test_merge(void)
{
    RUN_TEST(test_tables_merge);
}
