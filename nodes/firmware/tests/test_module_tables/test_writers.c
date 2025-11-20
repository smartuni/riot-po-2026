#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "container.h"
#include "store.h"
#include "crypto.h"
#include "unity.h"
#include "hlc_ztimer.h"

#include "hybrid_logical_clock.h"
#include "crypto_service.h"
#include "store_service.h"

#include "tables.h"
#include "tables/types.h"
#include "tables/keys.h"
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

static void _verify_store_header_call(store_put_call_info_t *store_call,
                                      const node_id_t *expected_writer,
                                      record_sequence_t expected_sequence,
                                      table_record_type_t expected_type,
                                      table_key_t *expected_key,
                                      table_record_t *record)
{
    memset(record, 0, sizeof(table_record_t));

    /* Check that the key is correct */
    make_header_key(expected_key);
    TEST_ASSERT_EQUAL_MEMORY(expected_key, store_call->key, TABLES_KEY_SIZE);
    TEST_ASSERT_EQUAL_INT(1, is_header_key(store_call->key));
    TEST_ASSERT_EQUAL_size_t(TABLES_KEY_SIZE, store_call->key_len);

    /* Check that the header is correct */
    table_record_header_t *header = (table_record_header_t *)store_call->data;
    record->header = *header;
    TEST_ASSERT_NOT_NULL(header);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_record_header_t), store_call->data_len);

    table_record_type_t record_type;
    get_record_type(record, &record_type);
    TEST_ASSERT_EQUAL_INT(expected_type, record_type);

    const node_id_t *record_writer;
    get_record_writer_id(record, &record_writer);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(*expected_writer, *record_writer, NODE_ID_SIZE);

    record_sequence_t sequence;
    get_record_sequence(record, &sequence);
    TEST_ASSERT_EQUAL_INT(expected_sequence, sequence);
}

static void _verify_store_data_key(store_put_call_info_t *store_call,
                                   table_key_t *expected_key)
{
    TEST_ASSERT_NOT_NULL(store_call->key);
    make_data_key(expected_key);
    TEST_ASSERT_EQUAL_MEMORY(expected_key, store_call->key, TABLES_KEY_SIZE);
    TEST_ASSERT_EQUAL_INT(1, is_data_key(store_call->key));
    TEST_ASSERT_EQUAL_size_t(TABLES_KEY_SIZE, store_call->key_len);
}

static void _verify_store_signature_length(store_put_call_info_t *store_call,
                                           table_key_t *expected_key)
{
    TEST_ASSERT_NOT_NULL(store_call->key);
    make_signature_length_key(expected_key);
    TEST_ASSERT_EQUAL_MEMORY(expected_key, store_call->key, TABLES_KEY_SIZE);
    TEST_ASSERT_EQUAL_INT(1, is_signature_length_key(store_call->key));
    TEST_ASSERT_EQUAL_size_t(TABLES_KEY_SIZE, store_call->key_len);

    /* Check that the signature length is correct */
    size_t *signature_len_ptr = (size_t *)store_call->data;
    TEST_ASSERT_NOT_NULL(signature_len_ptr);
    TEST_ASSERT_EQUAL_size_t(sizeof(size_t), store_call->data_len);
    TEST_ASSERT_EQUAL_size_t(DUMMY_SIGNATURE_LEN, *signature_len_ptr);
}

static void _verify_store_signature(store_put_call_info_t *store_call,
                                     table_key_t *expected_key)
{
    uint8_t expected_signature[DUMMY_SIGNATURE_LEN] = { DUMMY_SIGNATURE };
    size_t expected_signature_len = sizeof(expected_signature);

    TEST_ASSERT_NOT_NULL(store_call->key);
    make_signature_key(expected_key);
    TEST_ASSERT_EQUAL_MEMORY(expected_key, store_call->key, TABLES_KEY_SIZE);
    TEST_ASSERT_EQUAL_INT(1, is_signature_key(store_call->key));
    TEST_ASSERT_EQUAL_size_t(TABLES_KEY_SIZE, store_call->key_len);

    /* Check that the signature is correct */
    uint8_t *signature = (uint8_t *)store_call->data;
    TEST_ASSERT_NOT_NULL(signature);
    TEST_ASSERT_EQUAL_size_t(expected_signature_len, store_call->data_len);
    TEST_ASSERT_EQUAL_MEMORY(expected_signature, signature, expected_signature_len);
}

static void _verify_crypto_sign(crypto_sign_call_info_t *crypto_call,
                                const node_id_t *expected_signer,
                                uint8_t *expected_data,
                                size_t expected_data_len,
                                bool expect_signature_buffer)
{
    TEST_ASSERT_NOT_NULL(crypto_call->kid);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_signer, crypto_call->kid, NODE_ID_SIZE);

    TEST_ASSERT_NOT_NULL(crypto_call->data);
    TEST_ASSERT_EQUAL_size_t(expected_data_len, crypto_call->data_len);
    TEST_ASSERT_EQUAL_MEMORY(expected_data, crypto_call->data, expected_data_len);

    if (expect_signature_buffer) {
        TEST_ASSERT_NOT_NULL(crypto_call->signature);
        TEST_ASSERT_EQUAL_size_t(DUMMY_SIGNATURE_LEN, crypto_call->signature_len);
    } else {
        TEST_ASSERT_NULL(crypto_call->signature);
    }
}

void _verify_records_order(table_record_t *first, table_record_t *second)
{
    hlc_timestamp_t first_timestamp, second_timestamp;
    get_record_timestamp(first, &first_timestamp);
    get_record_timestamp(second, &second_timestamp);

    TEST_ASSERT_EQUAL_INT(-1, hlc_compare(&first_timestamp, &second_timestamp));
}

static void test_tables_put_gate_report(void)
{
    tables_context_t ctx;
    node_id_t self_id = { GATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    hlc_ztimer_t hlc_ztimer_ctx;
    store_ctx store_context;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &hlc_ztimer_ctx, &self_id);

    /* Add a first gate report record */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_OPEN));

    /* 4 calls: one for header, one for data, one for signature length, and one for
     * signature */
    TEST_ASSERT_EQUAL_UINT(4, store_context.calls);

    /* Call 0: the header */
    store_put_call_info_t *store_call_0;
    TEST_ASSERT_EQUAL_INT(0, store_get_put_call(&store_context, 0, &store_call_0));

    table_record_t record_0;
    table_key_t expected_key;
    get_gate_report_key(&self_id, &expected_key);
    _verify_store_header_call(store_call_0, &self_id, 1, RECORD_GATE_REPORT,
                              &expected_key, &record_0);

    /* Call 1: the data */
    store_put_call_info_t *store_call_1;
    store_get_put_call(&store_context, 1, &store_call_1);
    _verify_store_data_key(store_call_1, &expected_key);

    /* Check that the data is correct */
    const table_gate_report_t *data_0 = (table_gate_report_t *)store_call_1->data;
    TEST_ASSERT_NOT_NULL_MESSAGE(data_0, "Store call #1: data should not be NULL");
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_report_t), store_call_1->data_len);

    gate_state_t state_0;
    get_gate_report_state(data_0, &state_0);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_OPEN, state_0);

    /* Call 2: the signature length */
    store_put_call_info_t *store_call_2;
    store_get_put_call(&store_context, 2, &store_call_2);
    _verify_store_signature_length(store_call_2, &expected_key);

    /* Call 3: the signature */
    store_put_call_info_t *store_call_3;
    store_get_put_call(&store_context, 3, &store_call_3);
    _verify_store_signature(store_call_3, &expected_key);

    /* Call 0: without buffer */
    crypto_sign_call_info_t *crypto_call_0;
    crypto_get_sign_call(&crypto_context, 0, &crypto_call_0);

    /* Check that the passed data is correct */
    size_t crypto_data_len = sizeof(table_record_header_t) + sizeof(table_gate_report_t);
    uint8_t crypto_expected_data[crypto_data_len];
    memcpy(crypto_expected_data, &record_0.header, sizeof(table_record_header_t));
    memcpy(crypto_expected_data + sizeof(table_record_header_t),
           data_0, sizeof(table_gate_report_t));
    _verify_crypto_sign(crypto_call_0, &self_id, crypto_expected_data, crypto_data_len,
                        false);

    /* Call 1: with buffer */
    crypto_sign_call_info_t *crypto_call_1;
    crypto_get_sign_call(&crypto_context, 1, &crypto_call_1);
    _verify_crypto_sign(crypto_call_1, &self_id, crypto_expected_data, crypto_data_len,
                        true);

    /* Send new gate report to compare sequences and HLCs */
    TEST_ASSERT_EQUAL_INT(0, tables_put_gate_report(&ctx, GATE_STATE_CLOSED));

    /* Now we expect 4 more calls */
    TEST_ASSERT_EQUAL_UINT(8, store_context.calls);

    /* Recover the header of the new record */
    store_put_call_info_t *store_call_4;
    store_get_put_call(&store_context, 4, &store_call_4);

    table_record_t record_1;
    _verify_store_header_call(store_call_4, &self_id, 2, RECORD_GATE_REPORT,
                              &expected_key, &record_1);

    /* New record should be in the future */
    _verify_records_order(&record_0, &record_1);

    /* Recover the data of the new record */
    store_put_call_info_t *store_call_5;
    store_get_put_call(&store_context, 5, &store_call_5);
    _verify_store_data_key(store_call_5, &expected_key);

    /* Check that the data is correct */
    table_gate_report_t *data_1 = (table_gate_report_t *)store_call_5->data;
    TEST_ASSERT_NOT_NULL_MESSAGE(data_1, "Store call #5: data should not be NULL");
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_report_t), store_call_5->data_len);

    gate_state_t state_1;
    get_gate_report_state(data_1, &state_1);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_CLOSED, state_1);

    store_free_ctx(&store_context);
}

static void test_tables_put_gate_observation(void)
{
    tables_context_t ctx;
    node_id_t self_id = { MATE_TEST_ID };
    node_id_t gate_id = { GATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    hlc_ztimer_t hlc_ztimer_ctx;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_ctx store_context;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &hlc_ztimer_ctx, &self_id);

    /* Add a first gate observation record */
    TEST_ASSERT_EQUAL_INT(0,
        tables_put_gate_observation(&ctx, &gate_id, GATE_STATE_OPEN)
    );

    /* 4 calls: one for header, one for data, one for signature length, and one for
     * signature */
    TEST_ASSERT_EQUAL_UINT(4, store_context.calls);

    /* Call 0: the header */
    store_put_call_info_t *store_call_0;
    TEST_ASSERT_EQUAL_INT(0, store_get_put_call(&store_context, 0, &store_call_0));

    table_record_t record_0;
    table_key_t expected_key;
    get_gate_observation_key(&self_id, &gate_id, &expected_key);
    _verify_store_header_call(store_call_0, &self_id, 1, RECORD_GATE_OBSERVATION,
                              &expected_key, &record_0);

    /* Call 1: the data */
    store_put_call_info_t *store_call_1;
    store_get_put_call(&store_context, 1, &store_call_1);
    _verify_store_data_key(store_call_1, &expected_key);

    /* Check that the data is correct */
    const table_gate_observation_t *data_0 = (table_gate_observation_t *)store_call_1->data;
    TEST_ASSERT_NOT_NULL(data_0);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_observation_t), store_call_1->data_len);

    gate_state_t state_0;
    get_gate_observation_state(data_0, &state_0);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_OPEN, state_0);

    const node_id_t *gate_id_0;
    get_gate_observation_id(data_0, &gate_id_0);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gate_id, *gate_id_0, NODE_ID_SIZE);

    /* Call 2: the signature length */
    store_put_call_info_t *store_call_2;
    store_get_put_call(&store_context, 2, &store_call_2);
    _verify_store_signature_length(store_call_2, &expected_key);

    /* Call 3: the signature */
    store_put_call_info_t *store_call_3;
    store_get_put_call(&store_context, 3, &store_call_3);
    _verify_store_signature(store_call_3, &expected_key);

    /* Call 0: without buffer */
    crypto_sign_call_info_t *crypto_call_0;
    crypto_get_sign_call(&crypto_context, 0, &crypto_call_0);

    /* Check that the passed data is correct */
    size_t crypto_data_len = sizeof(table_record_header_t) + sizeof(table_gate_observation_t);
    uint8_t crypto_expected_data[crypto_data_len];
    memcpy(crypto_expected_data, &record_0.header, sizeof(table_record_header_t));
    memcpy(crypto_expected_data + sizeof(table_record_header_t),
           data_0, sizeof(table_gate_observation_t));
    _verify_crypto_sign(crypto_call_0, &self_id, crypto_expected_data, crypto_data_len,
                        false);

    /* Call 1: with buffer */
    crypto_sign_call_info_t *crypto_call_1;
    crypto_get_sign_call(&crypto_context, 1, &crypto_call_1);
    _verify_crypto_sign(crypto_call_1, &self_id, crypto_expected_data, crypto_data_len,
                        true);

    /* Send new gate observation to compare sequences and HLCs */
    TEST_ASSERT_EQUAL_INT(0,
        tables_put_gate_observation(&ctx, &gate_id, GATE_STATE_CLOSED)
    );

    /* Now we expect 4 more calls */
    TEST_ASSERT_EQUAL_UINT(8, store_context.calls);

    /* Recover the header of the new record */
    store_put_call_info_t *store_call_4;
    store_get_put_call(&store_context, 4, &store_call_4);

    table_record_t record_1;
    _verify_store_header_call(store_call_4, &self_id, 2, RECORD_GATE_OBSERVATION,
                              &expected_key, &record_1);

    /* New record should be in the future */
    _verify_records_order(&record_0, &record_1);

    /* Recover the data of the new record */
    store_put_call_info_t *store_call_5;
    store_get_put_call(&store_context, 5, &store_call_5);
    _verify_store_data_key(store_call_5, &expected_key);

    /* Check that the data is correct */
    table_gate_observation_t *data_1 = (table_gate_observation_t *)store_call_5->data;
    TEST_ASSERT_NOT_NULL(data_1);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_observation_t), store_call_5->data_len);

    gate_state_t state_1;
    get_gate_observation_state(data_1, &state_1);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_CLOSED, state_1);

    const node_id_t *gate_id_1;
    get_gate_observation_id(data_1, &gate_id_1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gate_id, *gate_id_1, NODE_ID_SIZE);

    store_free_ctx(&store_context);
}

static void test_tables_put_gate_encounter(void)
{
    tables_context_t ctx;
    node_id_t self_id = { MATE_TEST_ID };
    node_id_t gate_id = { GATE_TEST_ID };
    hlc_ctx_t hlc_ctx;
    hlc_ztimer_t hlc_ztimer_ctx;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_ctx store_context;
    rssi_t rssi = -42;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &hlc_ztimer_ctx, &self_id);

    /* Add a first gate encounter record */
    TEST_ASSERT_EQUAL_INT(0,
        tables_put_gate_encounter(&ctx, &gate_id, GATE_STATE_OPEN, rssi)
    );

    /* 4 calls: one for header, one for data, one for signature length, and one for
     * signature */
    TEST_ASSERT_EQUAL_UINT(4, store_context.calls);

    /* Store call 0: the header */
    store_put_call_info_t *store_call_0;
    TEST_ASSERT_EQUAL_INT(0, store_get_put_call(&store_context, 0, &store_call_0));

    table_record_t record_0;
    table_key_t expected_key;
    get_gate_encounter_key(&self_id, &gate_id, &expected_key);
    _verify_store_header_call(store_call_0, &self_id, 1, RECORD_GATE_ENCOUNTER,
                              &expected_key, &record_0);

    /* Store call 1: the data */
    store_put_call_info_t *store_call_1;
    store_get_put_call(&store_context, 1, &store_call_1);
    _verify_store_data_key(store_call_1, &expected_key);

    /* Check that the data is correct */
    const table_gate_encounter_t *data_0 = (table_gate_encounter_t *)store_call_1->data;
    TEST_ASSERT_NOT_NULL(data_0);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_encounter_t), store_call_1->data_len);

    gate_state_t state_0;
    get_gate_encounter_state(data_0, &state_0);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_OPEN, state_0);

    const node_id_t *gate_id_0;
    get_gate_encounter_id(data_0, &gate_id_0);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gate_id, *gate_id_0, NODE_ID_SIZE);

    rssi_t rssi_0;
    get_gate_encounter_rssi(data_0, &rssi_0);
    TEST_ASSERT_EQUAL_INT(rssi, rssi_0);

    /* Store call 2: the signature length */
    store_put_call_info_t *store_call_2;
    store_get_put_call(&store_context, 2, &store_call_2);
    _verify_store_signature_length(store_call_2, &expected_key);

    /* Store call 3: the signature */
    store_put_call_info_t *store_call_3;
    store_get_put_call(&store_context, 3, &store_call_3);
    _verify_store_signature(store_call_3, &expected_key);

    /* Store call 0: without buffer */
    crypto_sign_call_info_t *crypto_call_0;
    crypto_get_sign_call(&crypto_context, 0, &crypto_call_0);
    /* Check that the passed data is correct */
    size_t crypto_data_len = sizeof(table_record_header_t) + sizeof(table_gate_encounter_t);
    uint8_t crypto_expected_data[crypto_data_len];
    memcpy(crypto_expected_data, &record_0.header, sizeof(table_record_header_t));
    memcpy(crypto_expected_data + sizeof(table_record_header_t),
           data_0, sizeof(table_gate_encounter_t));
    _verify_crypto_sign(crypto_call_0, &self_id, crypto_expected_data, crypto_data_len,
                        false);

    /* Store call 1: with buffer */
    crypto_sign_call_info_t *crypto_call_1;
    crypto_get_sign_call(&crypto_context, 1, &crypto_call_1);
    _verify_crypto_sign(crypto_call_1, &self_id, crypto_expected_data, crypto_data_len,
                        true);

    /* Send new gate observation to compare sequences and HLCs */
    rssi = -30;
    TEST_ASSERT_EQUAL_INT(0,
        tables_put_gate_encounter(&ctx, &gate_id, GATE_STATE_CLOSED, rssi)
    );

    /* Now we expect 4 more calls */
    TEST_ASSERT_EQUAL_UINT(8, store_context.calls);

    /* Recover the header of the new record */
    store_put_call_info_t *store_call_4;
    store_get_put_call(&store_context, 4, &store_call_4);

    table_record_t record_1;
    _verify_store_header_call(store_call_4, &self_id, 2, RECORD_GATE_ENCOUNTER,
                              &expected_key, &record_1);

    /* New record should be in the future */
    _verify_records_order(&record_0, &record_1);

    /* Recover the data of the new record */
    store_put_call_info_t *store_call_5;
    store_get_put_call(&store_context, 5, &store_call_5);
    _verify_store_data_key(store_call_5, &expected_key);

    /* Check that the data is correct */
    table_gate_encounter_t *data_1 = (table_gate_encounter_t *)store_call_5->data;
    TEST_ASSERT_NOT_NULL(data_1);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_gate_encounter_t), store_call_5->data_len);

    gate_state_t state_1;
    get_gate_encounter_state(data_1, &state_1);
    TEST_ASSERT_EQUAL_INT(GATE_STATE_CLOSED, state_1);

    const node_id_t *gate_id_1;
    get_gate_encounter_id(data_1, &gate_id_1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(gate_id, *gate_id_1, NODE_ID_SIZE);

    rssi_t rssi_1;
    get_gate_encounter_rssi(data_1, &rssi_1);
    TEST_ASSERT_EQUAL_INT(rssi, rssi_1);

    store_free_ctx(&store_context);
}

static void test_tables_put_mate_encounter(void)
{
    tables_context_t ctx;
    node_id_t self_id = { MATE_TEST_ID };
    node_id_t mate_id = { MATE_2_TEST_ID };
    hlc_ctx_t hlc_ctx;
    hlc_ztimer_t hlc_ztimer_ctx;
    crypto_ctx_t crypto_context;
    crypto_service_t crypto_service;
    store_ctx store_context;
    rssi_t rssi = -42;
    store_service_t store_service;

    _setup_test(&ctx, &store_context, &store_service, &crypto_context, &crypto_service,
                &hlc_ctx, &hlc_ztimer_ctx, &self_id);

    /* Add a first mate encounter record */
    TEST_ASSERT_EQUAL_INT(0, tables_put_mate_encounter(&ctx, &mate_id, rssi));

    /* 4 calls: one for header, one for data, one for signature length, and one for
     * signature */
    TEST_ASSERT_EQUAL_UINT(4, store_context.calls);

    /* Store call 0: the header */
    store_put_call_info_t *store_call_0;
    TEST_ASSERT_EQUAL_INT(0, store_get_put_call(&store_context, 0, &store_call_0));

    table_record_t record_0;
    table_key_t expected_key;
    get_mate_encounter_key(&self_id, &mate_id, &expected_key);
    _verify_store_header_call(store_call_0, &self_id, 1, RECORD_MATE_ENCOUNTER,
                              &expected_key, &record_0);

    /* Store call 1: the data */
    store_put_call_info_t *store_call_1;
    store_get_put_call(&store_context, 1, &store_call_1);
    _verify_store_data_key(store_call_1, &expected_key);

    /* Check that the data is correct */
    const table_mate_encounter_t *data_0 = (table_mate_encounter_t *)store_call_1->data;
    TEST_ASSERT_NOT_NULL(data_0);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_mate_encounter_t), store_call_1->data_len);

    const node_id_t *mate_id_0;
    get_mate_encounter_id(data_0, &mate_id_0);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(mate_id, *mate_id_0, NODE_ID_SIZE);

    rssi_t rssi_0;
    get_mate_encounter_rssi(data_0, &rssi_0);
    TEST_ASSERT_EQUAL_INT(rssi, rssi_0);

    /* Store call 2: the signature length */
    store_put_call_info_t *store_call_2;
    store_get_put_call(&store_context, 2, &store_call_2);
    _verify_store_signature_length(store_call_2, &expected_key);

    /* Store call 3: the signature */
    store_put_call_info_t *store_call_3;
    store_get_put_call(&store_context, 3, &store_call_3);
    _verify_store_signature(store_call_3, &expected_key);

    /* Store call 0: without buffer */
    crypto_sign_call_info_t *crypto_call_0;
    crypto_get_sign_call(&crypto_context, 0, &crypto_call_0);
    /* Check that the passed data is correct */
    size_t crypto_data_len = sizeof(table_record_header_t) + sizeof(table_mate_encounter_t);
    uint8_t crypto_expected_data[crypto_data_len];
    memcpy(crypto_expected_data, &record_0.header, sizeof(table_record_header_t));
    memcpy(crypto_expected_data + sizeof(table_record_header_t),
           data_0, sizeof(table_mate_encounter_t));
    _verify_crypto_sign(crypto_call_0, &self_id, crypto_expected_data, crypto_data_len,
                        false);

    /* Store call 1: with buffer */
    crypto_sign_call_info_t *crypto_call_1;
    crypto_get_sign_call(&crypto_context, 1, &crypto_call_1);
    _verify_crypto_sign(crypto_call_1, &self_id, crypto_expected_data, crypto_data_len,
                        true);

    /* Send new mate encounter to compare sequences and HLCs */
    rssi = -30;
    TEST_ASSERT_EQUAL_INT(0, tables_put_mate_encounter(&ctx, &mate_id, rssi));

    /* Now we expect 4 more calls */
    TEST_ASSERT_EQUAL_UINT(8, store_context.calls);

    /* Recover the header of the new record */
    store_put_call_info_t *store_call_4;
    store_get_put_call(&store_context, 4, &store_call_4);

    table_record_t record_1;
    _verify_store_header_call(store_call_4, &self_id, 2, RECORD_MATE_ENCOUNTER,
                              &expected_key, &record_1);

    /* New record should be in the future */
    _verify_records_order(&record_0, &record_1);

    /* Recover the data of the new record */
    store_put_call_info_t *store_call_5;
    store_get_put_call(&store_context, 5, &store_call_5);
    _verify_store_data_key(store_call_5, &expected_key);

    /* Check that the data is correct */
    table_mate_encounter_t *data_1 = (table_mate_encounter_t *)store_call_5->data;
    TEST_ASSERT_NOT_NULL(data_1);
    TEST_ASSERT_EQUAL_size_t(sizeof(table_mate_encounter_t), store_call_5->data_len);

    const node_id_t *mate_id_1;
    get_mate_encounter_id(data_1, &mate_id_1);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(mate_id, *mate_id_1, NODE_ID_SIZE);

    rssi_t rssi_1;
    get_mate_encounter_rssi(data_1, &rssi_1);
    TEST_ASSERT_EQUAL_INT(rssi, rssi_1);

    store_free_ctx(&store_context);
}

void test_writers(void)
{
    RUN_TEST(test_tables_put_gate_report);
    RUN_TEST(test_tables_put_gate_observation);
    RUN_TEST(test_tables_put_gate_encounter);
    RUN_TEST(test_tables_put_mate_encounter);
}
