#include <string.h>

#include "unity.h"
#include "cbor_serialization.h"
#include "tables/types.h"
#include "od.h"

#include "test_payloads.h"


static void test_cbor_serialize_gate_observation(void)
{
    size_t out_len = 128;
    uint8_t out[out_len];

    table_record_t record;
    node_id_t id = {0x01, 0x02, 0x03, 0x04};
    memcpy(record.header.writer, id, NODE_ID_SIZE);

    node_id_t gate_id = {0x11, 0x12, 0x13, 0x14};

    record.header.type = RECORD_GATE_OBSERVATION;
    record.header.sequence = 1;
    record.header.timestamp.physical = 0xAAFF;
    record.header.timestamp.logical = 0xFFFF;

    table_gate_observation_t observation = {
        .state = GATE_STATE_CLOSED,
    };

    record.data.gate_observation = &observation;
    memcpy(observation.gate_id, gate_id, NODE_ID_SIZE);

    uint8_t signature[75];
    memset(signature, 0xFF, sizeof(signature));

    record.signature_len = sizeof(signature);
    record.signature = signature;

    TEST_ASSERT_EQUAL_INT(0, cbor_serialize_record(&record, out, &out_len));

    TEST_ASSERT_EQUAL_MEMORY(_gate_observation, out, out_len);
}

static void test_cbor_serialize_gate_report(void)
{
    size_t out_len = 128;
    uint8_t out[out_len];

    table_record_t record;
    node_id_t id = {0x01, 0x02, 0x03, 0x04};
    memcpy(record.header.writer, id, NODE_ID_SIZE);

    record.header.type = RECORD_GATE_REPORT;
    record.header.sequence = 1;
    record.header.timestamp.physical = 0xAAFF;
    record.header.timestamp.logical = 0xFFFF;

    table_gate_report_t report = {
        .state = GATE_STATE_CLOSED,
    };

    record.data.gate_report = &report;

    uint8_t signature[75];
    memset(signature, 0xFF, sizeof(signature));

    record.signature_len = sizeof(signature);
    record.signature = signature;

    TEST_ASSERT_EQUAL_INT(0, cbor_serialize_record(&record, out, &out_len));

    TEST_ASSERT_EQUAL_MEMORY(_gate_report, out, out_len);
}

static void test_cbor_serialize_gate_job(void)
{
    size_t out_len = 128;
    uint8_t out[out_len];

    table_record_t record;
    node_id_t id = {0x01, 0x02, 0x03, 0x04};
    memcpy(record.header.writer, id, NODE_ID_SIZE);

    node_id_t gate_id = {0x11, 0x12, 0x13, 0x14};

    record.header.type = RECORD_GATE_JOB;
    record.header.sequence = 1;
    record.header.timestamp.physical = 0xAABB;
    record.header.timestamp.logical = 0x0A;

    table_gate_job_t job = {
        .state = GATE_STATE_CLOSED,
    };
    memcpy(job.gate_id, gate_id, NODE_ID_SIZE);
    record.data.gate_job = &job;

    memcpy(job.assigned_device, id, NODE_ID_SIZE);
    record.data.gate_job = &job;

    uint8_t signature[75];
    memset(signature, 0xFF, sizeof(signature));

    record.signature_len = sizeof(signature);
    record.signature = signature;

    TEST_ASSERT_EQUAL_INT(0, cbor_serialize_record(&record, out, &out_len));

    TEST_ASSERT_EQUAL_MEMORY(_gate_job, out, out_len);
}

static void test_cbor_serialize_gate_command(void)
{
    size_t out_len = 128;
    uint8_t out[out_len];

    table_record_t record;
    node_id_t id = {0x01, 0x02, 0x03, 0x04};
    memcpy(record.header.writer, id, NODE_ID_SIZE);

    node_id_t gate_id = {0x11, 0x12, 0x13, 0x14};

    record.header.type = RECORD_GATE_COMMAND;
    record.header.sequence = 1;
    record.header.timestamp.physical = 0xDACB;
    record.header.timestamp.logical = 0x0A;

    table_gate_command_t command = {
        .state = GATE_STATE_CLOSED,
    };
    memcpy(command.gate_id, gate_id, NODE_ID_SIZE);
    record.data.gate_command = &command;

    uint8_t signature[75];
    memset(signature, 0xFF, sizeof(signature));

    record.signature_len = sizeof(signature);
    record.signature = signature;

    TEST_ASSERT_EQUAL_INT(0, cbor_serialize_record(&record, out, &out_len));

    TEST_ASSERT_EQUAL_MEMORY(_gate_command, out, out_len);
}

void test_serialize(void)
{
    RUN_TEST(test_cbor_serialize_gate_job);
    RUN_TEST(test_cbor_serialize_gate_report);
    RUN_TEST(test_cbor_serialize_gate_observation);
    RUN_TEST(test_cbor_serialize_gate_command);
}
