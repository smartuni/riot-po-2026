#include <stdint.h>

#define INVALID_GATE_ID (0xFF)
#define MAX_GATE_COUNT (0xFF)

typedef struct {
    uint8_t gateID;
    uint8_t state;
    int timestamp;
} target_state_entry;

typedef struct {
    uint8_t gateID;
    uint8_t state;
    int gateTime;
} is_state_entry;

typedef struct {
    uint8_t gateID;
    int gateTime;
    uint8_t status;
    uint8_t senseMateID;
} seen_status_entry;

typedef struct {
    uint8_t gateID;
} jobs_entry;

typedef struct {
    uint8_t* buffer;
    int cbor_size;
    int capacity;
} cbor_buffer;

int target_state_table_to_cbor(target_state_entry table[MAX_GATE_COUNT], cbor_buffer buffer);

int is_state_table_to_cbor(is_state_entry table[MAX_GATE_COUNT], cbor_buffer buffer);

int seen_status_table_to_cbor(seen_status_entry table[MAX_GATE_COUNT], cbor_buffer buffer);

int jobs_table_to_cbor(jobs_entry table[MAX_GATE_COUNT], cbor_buffer buffer);

int target_state_entry_to_cbor(target_state_entry entry, cbor_buffer buffer);

int is_state_entry_to_cbor(is_state_entry entry, cbor_buffer buffer);

int seen_status_entry_to_cbor(seen_status_entry entry, cbor_buffer buffer);

int jobs_entry_to_cbor(jobs_entry entry, cbor_buffer buffer);
