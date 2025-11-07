#include <stdint.h>
#include <string.h>

#include "unity.h"
#include "cbor_serialization.h"
#include "tables/types.h"
#include "od.h"

#include "test_payloads.h"


static void test_cbor_deserialize_gate_observation(void)
{
    table_record_t record;
    table_record_data_buffer_t record_data;
    size_t signature_len = sizeof(_dummy_signature);
    uint8_t signature[signature_len];

    TEST_ASSERT_EQUAL_INT(0,
        cbor_deserialize(_gate_observation, sizeof(_gate_observation), &record,
                         &record_data, signature, &signature_len)
    );

    // Check header type
    TEST_ASSERT_EQUAL(RECORD_GATE_OBSERVATION, record.header.type);

    // Check writer ID
    node_id_t expected_writer = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL_MEMORY(expected_writer, record.header.writer, NODE_ID_SIZE);

    // Check sequence number
    TEST_ASSERT_EQUAL_UINT64(1, record.header.sequence);

    // Check timestamps
    TEST_ASSERT_EQUAL_UINT64(0xAAFF, record.header.timestamp.physical);
    TEST_ASSERT_EQUAL_UINT64(0xFFFF, record.header.timestamp.logical);

    // Check gate ID in observation data
    node_id_t expected_gate_id = {0x11, 0x12, 0x13, 0x14};
    TEST_ASSERT_EQUAL_MEMORY(expected_gate_id, record_data.gate_observation.gate_id, NODE_ID_SIZE);

    // Check gate state
    TEST_ASSERT_EQUAL(GATE_STATE_CLOSED, record_data.gate_observation.state);

    // Check signature
    TEST_ASSERT_EQUAL_MEMORY(_dummy_signature, signature, signature_len);
}

static void test_cbor_deserialize_gate_report(void)
{
    table_record_t record;
    table_record_data_buffer_t record_data;
    size_t signature_len = sizeof(_dummy_signature);
    uint8_t signature[signature_len];

    TEST_ASSERT_EQUAL_INT(0,
        cbor_deserialize(_gate_report, sizeof(_gate_report), &record,
                         &record_data, signature, &signature_len)
    );

    // Check header type
    TEST_ASSERT_EQUAL(RECORD_GATE_REPORT, record.header.type);

    // Check writer ID
    node_id_t expected_writer = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL_MEMORY(expected_writer, record.header.writer, NODE_ID_SIZE);

    // Check sequence number
    TEST_ASSERT_EQUAL_UINT64(1, record.header.sequence);

    // Check timestamps
    TEST_ASSERT_EQUAL_UINT64(0xAAFF, record.header.timestamp.physical);
    TEST_ASSERT_EQUAL_UINT64(0xFFFF, record.header.timestamp.logical);

    // Check gate state
    TEST_ASSERT_EQUAL(GATE_STATE_CLOSED, record_data.gate_report.state);

    // Check signature
    TEST_ASSERT_EQUAL_MEMORY(_dummy_signature, signature, signature_len);
}

static void test_cbor_deserialize_gate_job(void)
{
    table_record_t record;
    table_record_data_buffer_t record_data;
    size_t signature_len = sizeof(_dummy_signature);
    uint8_t signature[signature_len];

    TEST_ASSERT_EQUAL_INT(0,
        cbor_deserialize(_gate_job, sizeof(_gate_job), &record,
                         &record_data, signature, &signature_len)
    );

    // Check header type
    TEST_ASSERT_EQUAL(RECORD_GATE_JOB, record.header.type);

    // Check writer ID
    node_id_t expected_writer = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL_MEMORY(expected_writer, record.header.writer, NODE_ID_SIZE);

    // Check sequence number
    TEST_ASSERT_EQUAL_UINT64(1, record.header.sequence);

    // Check timestamps
    TEST_ASSERT_EQUAL_UINT64(0xAABB, record.header.timestamp.physical);
    TEST_ASSERT_EQUAL_UINT64(0x0A, record.header.timestamp.logical);

    // Check gate ID in job data
    node_id_t expected_gate_id = {0x11, 0x12, 0x13, 0x14};
    TEST_ASSERT_EQUAL_MEMORY(expected_gate_id, record_data.gate_job.gate_id, NODE_ID_SIZE);

    // Check gate state
    TEST_ASSERT_EQUAL(GATE_STATE_CLOSED, record_data.gate_job.state);

    // Check assigned device
    node_id_t expected_assigned_device = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL_MEMORY(expected_assigned_device,
                             record_data.gate_job.assigned_device, NODE_ID_SIZE);

    // Check signature
    TEST_ASSERT_EQUAL_MEMORY(_dummy_signature, signature, signature_len);
}

static void test_cbor_deserialize_gate_command(void)
{
    table_record_t record;
    table_record_data_buffer_t record_data;
    size_t signature_len = sizeof(_dummy_signature);
    uint8_t signature[signature_len];

    TEST_ASSERT_EQUAL_INT(0,
        cbor_deserialize(_gate_command, sizeof(_gate_command), &record,
                         &record_data, signature, &signature_len)
    );

    // Check header type
    TEST_ASSERT_EQUAL(RECORD_GATE_COMMAND, record.header.type);

    // Check writer ID
    node_id_t expected_writer = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL_MEMORY(expected_writer, record.header.writer, NODE_ID_SIZE);

    // Check sequence number
    TEST_ASSERT_EQUAL_UINT64(1, record.header.sequence);

    // Check timestamps
    TEST_ASSERT_EQUAL_UINT64(0xDACB, record.header.timestamp.physical);
    TEST_ASSERT_EQUAL_UINT64(0x0A, record.header.timestamp.logical);

    // Check gate ID in command data
    node_id_t expected_gate_id = {0x11, 0x12, 0x13, 0x14};
    TEST_ASSERT_EQUAL_MEMORY(expected_gate_id, record_data.gate_command.gate_id, NODE_ID_SIZE);

    // Check gate state
    TEST_ASSERT_EQUAL(GATE_STATE_CLOSED, record_data.gate_command.state);

    // Check signature
    TEST_ASSERT_EQUAL_MEMORY(_dummy_signature, signature, signature_len);
}

void test_deserialize(void)
{
    RUN_TEST(test_cbor_deserialize_gate_observation);
    RUN_TEST(test_cbor_deserialize_gate_report);
    RUN_TEST(test_cbor_deserialize_gate_job);
    RUN_TEST(test_cbor_deserialize_gate_command);
}
