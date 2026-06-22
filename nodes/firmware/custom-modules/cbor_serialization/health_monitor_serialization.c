#include "health_monitor_serialization.h"


static int _serialize_with_array_encoding(const health_monitor_payload_t* record, uint8_t* buf, size_t* out_len) {
	CborEncoder root_encoder;
	cbor_encoder_init(&root_encoder, buf, *out_len, 0);
	CborEncoder main_array_encoder;
	cbor_encoder_create_array(&root_encoder, &main_array_encoder, HEALTH_MONITOR_PAYLOAD_ELEMENTS+3); // +3 for version, msg_type, and node_id

    cbor_encode_uint(&main_array_encoder, ENCODING_V_1);
    cbor_encode_uint(&main_array_encoder, MESSAGE_TYPE_HEALTH_MONITOR);
    uint16_t node_id = 0; // Placeholder for node_id, you can replace it with actual value if needed
    cbor_encode_uint(&main_array_encoder, node_id);
	cbor_encode_uint(&main_array_encoder, record->shock_status);
	cbor_encode_uint(&main_array_encoder, record->battery_info.battery_status);
	cbor_encode_uint(&main_array_encoder, record->battery_info.voltage_mv);
	cbor_encoder_close_container(&root_encoder, &main_array_encoder);

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, buf);
    return 0;
}

static int _serialize_without_array_encoding(const health_monitor_payload_t* record, uint8_t* buf, size_t* out_len) {
    CborEncoder encoder;
    cbor_encoder_init(&encoder, buf, *out_len, 0);

    cbor_encode_uint(&encoder, ENCODING_V_1);
    cbor_encode_uint(&encoder, MESSAGE_TYPE_HEALTH_MONITOR);
    int node_id = 0; // Placeholder for node_id, you can replace it with actual value if needed
    cbor_encode_uint(&encoder, node_id);
    cbor_encode_uint(&encoder, record->shock_status);
    cbor_encode_uint(&encoder, record->battery_info.battery_status);
    cbor_encode_uint(&encoder, record->battery_info.voltage_mv);

    *out_len = cbor_encoder_get_buffer_size(&encoder, buf);
    return 0;
}

int health_monitor_serialize_record_no_sig(const health_monitor_payload_t* record, uint8_t* out, size_t* out_len, bool use_array) {
    if(use_array){
        return _serialize_with_array_encoding(record, out, out_len);
    }
    return _serialize_without_array_encoding(record, out, out_len);

}