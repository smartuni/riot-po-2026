#include "health_monitor_serialization.h"

#define ENABLE_DEBUG (0)
#include "debug.h"


int health_monitor_serialize(const health_monitor_payload_t* record, uint8_t* buf, size_t* out_len) {
    assert(record != NULL);
    assert(buf !=  NULL);
    assert(out_len != NULL);

    CborEncoder root_encoder;
	cbor_encoder_init(&root_encoder, buf, *out_len, 0);
	CborEncoder main_array_encoder;
    int result;
	result = cbor_encoder_create_array(&root_encoder, &main_array_encoder, HEALTH_MONITOR_CBOR_SIZE_COUNT); // +3 for version, msg_type, and node_id
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to create CBOR array\n", __LINE__);
    }

    result = cbor_encode_uint(&main_array_encoder, ENCODING_V_1);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to encode version\n", __LINE__);
    }
    result = cbor_encode_uint(&main_array_encoder, MESSAGE_TYPE_HEALTH_MONITOR);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to encode message type\n", __LINE__);
    }
    uint32_t node_id = (uint32_t)self_node_id;
    result = cbor_encode_uint(&main_array_encoder, node_id);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to encode node ID\n", __LINE__);
    }
    result = cbor_encode_uint(&main_array_encoder, record->header);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to encode header\n", __LINE__);
    }
    result = cbor_encode_uint(&main_array_encoder, record->body);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to encode body\n", __LINE__);
    }

	result = cbor_encoder_close_container(&root_encoder, &main_array_encoder);
    if(result != 0){
        DEBUG("[health_monitor_serialization.c:%d] Failed to close CBOR array\n", __LINE__);
    }

    *out_len = cbor_encoder_get_buffer_size(&root_encoder, buf);
    return 0;
}