#include "unity.h"

#include "tables/keys.h"

#define GATE_TEST_ID 0x01, 0x02, 0x03, 0x04
#define MATE_TEST_ID 0x05, 0x06, 0x07, 0x08

static void test_get_gate_report_key(void)
{
    table_key_t key;
    node_id_t gate = {GATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_GATE_REPORT, KEY_TYPE_HEADER, GATE_TEST_ID,
                              0x00, 0x00, 0x00, 0x00};

    get_gate_report_key(&gate, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

static void test_get_gate_observation_key(void)
{
    table_key_t key;
    node_id_t gate = {GATE_TEST_ID};
    node_id_t observer = {MATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_GATE_OBSERVATION, KEY_TYPE_HEADER, MATE_TEST_ID,
                              GATE_TEST_ID};

    get_gate_observation_key(&observer, &gate, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

static void test_get_gate_encounter_key(void)
{
    table_key_t key;
    node_id_t gate = {GATE_TEST_ID};
    node_id_t observer = {MATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_GATE_ENCOUNTER, KEY_TYPE_HEADER, MATE_TEST_ID,
                              GATE_TEST_ID};

    get_gate_encounter_key(&observer, &gate, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);

    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

static void test_get_mate_encounter_key(void)
{
    table_key_t key;
    node_id_t mate = {MATE_TEST_ID};
    node_id_t observer = {GATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_MATE_ENCOUNTER, KEY_TYPE_HEADER, GATE_TEST_ID,
                              MATE_TEST_ID};

    get_mate_encounter_key(&observer, &mate, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

static void test_get_gate_command_key(void)
{
    table_key_t key;
    node_id_t gate = {GATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_GATE_COMMAND, KEY_TYPE_HEADER, GATE_TEST_ID,
                              0x00, 0x00, 0x00, 0x00};

    get_gate_command_key(&gate, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);

    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

static void test_get_gate_job_key(void)
{
    table_key_t key;
    node_id_t gate = {GATE_TEST_ID};
    node_id_t device = {MATE_TEST_ID};
    uint8_t expected_key[] = {RECORD_GATE_JOB, KEY_TYPE_HEADER, GATE_TEST_ID,
                              MATE_TEST_ID};

    get_gate_job_key(&gate, &device, &key);

    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_data_key(&key);
    TEST_ASSERT_EQUAL_INT(is_data_key(&key), 1);

    expected_key[1] = KEY_TYPE_DATA;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_key(&key), 1);

    expected_key[1] = KEY_TYPE_SIGNATURE;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );

    make_signature_length_key(&key);
    TEST_ASSERT_EQUAL_INT(is_signature_length_key(&key), 1);
    expected_key[1] = KEY_TYPE_SIGNATURE_LENGTH;
    TEST_ASSERT_EQUAL_MEMORY(
        expected_key,
        key,
        TABLES_KEY_SIZE
    );
}

void test_keys(void)
{
    RUN_TEST(test_get_gate_report_key);
    RUN_TEST(test_get_gate_observation_key);
    RUN_TEST(test_get_gate_encounter_key);
    RUN_TEST(test_get_mate_encounter_key);
    RUN_TEST(test_get_gate_command_key);
    RUN_TEST(test_get_gate_job_key);
}
