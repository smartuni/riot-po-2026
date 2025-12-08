#include <string.h>

#include "assert.h"
#include "flashdb.h"

#include "base64.h"
#include "store_service.h"

#include "flashdb_store_service.h"

#define LOG_LEVEL   LOG_NONE
#include "log.h"
#define _LOGDBG(...) LOG_DEBUG("[flashdb_store_service]: " __VA_ARGS__)

static const char *ok(bool condition)
{
    return condition ? "[OK]" : "[ERROR]";
}

static void _lock_callback(fdb_db_t db)
{
    assert(db != NULL);
    flashdb_store_service_ctx_t *ctx = db->user_data;
    mutex_lock(&ctx->fdb_lock);
}

static void _unlock_callback(fdb_db_t db)
{
    assert(db != NULL);
    flashdb_store_service_ctx_t *ctx = db->user_data;
    mutex_unlock(&ctx->fdb_lock);
}

int flashdb_store_service_init(flashdb_store_service_ctx_t *ctx, const char *db_name,
                               const char *db_dir)
{
    assert(ctx != NULL);
    fdb_err_t result;

    memset(&ctx->kvdb, 0, sizeof(struct fdb_kvdb));

    if (IS_USED(MODULE_FLASHDB_VFS)) {
        /* enable file mode */
        bool file_mode = true;
        fdb_kvdb_control(&ctx->kvdb, FDB_KVDB_CTRL_SET_FILE_MODE, &file_mode);
    }

    mutex_init(&ctx->service_lock);

    /* set the lock and unlock callbacks */
    mutex_init(&ctx->fdb_lock);
    fdb_kvdb_control(&ctx->kvdb, FDB_KVDB_CTRL_SET_LOCK, _lock_callback);
    fdb_kvdb_control(&ctx->kvdb, FDB_KVDB_CTRL_SET_UNLOCK, _unlock_callback);

    /* set sizes */
    uint32_t sec_size = FLASHDB_STORE_SERVICE_SECTOR_SIZE;
    fdb_kvdb_control(&ctx->kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, &sec_size);
    uint32_t size = FLASHDB_STORE_SERVICE_DB_SIZE;
    fdb_kvdb_control(&ctx->kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, &size);

    result = fdb_kvdb_init(&ctx->kvdb, db_name, db_dir, NULL, ctx);
    if (result != FDB_NO_ERR) {
        return -1;
    }

    return 0;
}

static int _put_callback(const void *context, const uint8_t *key, size_t key_len,
                         const void *data, size_t data_len)
{
    assert(context != NULL);
    assert(key != NULL);
    assert(data != NULL);
    _LOGDBG("%s\n", __func__);
    flashdb_store_service_ctx_t *ctx = (flashdb_store_service_ctx_t *)context;
    fdb_err_t result;
    struct fdb_blob blob;
    mutex_lock(&ctx->service_lock);

    size_t key_b64_len = base64_estimate_encode_size(key_len);
    char key_b64[key_b64_len + 1];
    _LOGDBG("%s b64 estimated key len: %zu\n", __func__, key_b64_len);

    int res = base64_encode(key, key_len, key_b64, &key_b64_len);
    _LOGDBG("%s b64 encode %s\n", __func__, ok(res == BASE64_SUCCESS));
    if (res != BASE64_SUCCESS) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }
    key_b64[key_b64_len] = '\0';

    /* Create a blob from the data */
    fdb_blob_make(&blob, data, data_len);

    /* Store the blob in the KVDB */
    result = fdb_kv_set_blob(&ctx->kvdb, key_b64, &blob);
    _LOGDBG("%s set blob %s\n", __func__, ok(result == FDB_NO_ERR));
    if (result != FDB_NO_ERR) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }
    mutex_unlock(&ctx->service_lock);
    return 0;
}

static int _get_callback(const void *context, const uint8_t *key, size_t key_len,
                         void *out_data, size_t data_len)
{
    assert(context != NULL);
    assert(key != NULL);
    assert(out_data != NULL);

    flashdb_store_service_ctx_t *ctx = (flashdb_store_service_ctx_t *)context;
    struct fdb_blob blob;
    mutex_lock(&ctx->service_lock);

    size_t key_b64_len = base64_estimate_encode_size(key_len);
    char key_b64[key_b64_len + 1];

    int result = base64_encode(key, key_len, key_b64, &key_b64_len);
    if (result != BASE64_SUCCESS) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }
    key_b64[key_b64_len] = '\0';

    /* Prepare a blob to receive the data */
    fdb_blob_make(&blob, out_data, data_len);

    /* Retrieve the blob from the KVDB */
    fdb_kv_get_blob(&ctx->kvdb, key_b64, &blob);
    if (blob.saved.len == 0) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }

    mutex_unlock(&ctx->service_lock);
    return 0;
}

static int _delete_callback(const void *context, const uint8_t *key, size_t key_len)
{
    assert(context != NULL);
    assert(key != NULL);

    flashdb_store_service_ctx_t *ctx = (flashdb_store_service_ctx_t *)context;
    mutex_lock(&ctx->service_lock);

    size_t key_b64_len = base64_estimate_encode_size(key_len);
    char key_b64[key_b64_len + 1];

    int result = base64_encode(key, key_len, key_b64, &key_b64_len);
    if (result != BASE64_SUCCESS) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }
    key_b64[key_b64_len] = '\0';

    fdb_err_t res = fdb_kv_del(&ctx->kvdb, key_b64);
    if (res != FDB_NO_ERR) {
        mutex_unlock(&ctx->service_lock);
        return -1;
    }

    mutex_unlock(&ctx->service_lock);
    return 0;
}

static int _iterate_next_callback(const void *context,
                                  store_service_iterator_t *iterator, uint8_t *key,
                                  size_t *key_len, void *data, size_t *data_len)
{
    assert(context != NULL);
    assert(iterator != NULL);
    assert(key != NULL);
    assert(data != NULL);
    assert(data_len != NULL);
    assert(*data_len > 0);
    assert(key_len != NULL);
    assert(*key_len > 0);

    // it's our context, so it's fine to discard the const qualifier
    flashdb_store_service_ctx_t *ctx = (flashdb_store_service_ctx_t *)context;
    flashdb_store_service_iterator_t *flashdb_iterator =
        (flashdb_store_service_iterator_t *)iterator;

    mutex_lock(&ctx->service_lock);
    int res = -1;
    while (fdb_kv_iterate(&ctx->kvdb, &flashdb_iterator->iterator)) {
        fdb_kv_t kv = &(flashdb_iterator->iterator.curr_kv);
        uint8_t *key_b64 = (uint8_t *)kv->name;
        size_t key_b64_len = kv->name_len - 1;

        size_t _key_len = base64_estimate_decode_size(key_b64_len);
        uint8_t _key[_key_len];
        int result = base64_decode(key_b64, key_b64_len, _key, &_key_len);
        if (result != BASE64_SUCCESS) {
            res = -2;
            break;
        }

        if (store_service_key_matches_query(_key, _key_len, &flashdb_iterator->query)) {
            struct fdb_blob blob;

            // Check if we have enough space to read the blob and key
            if (*data_len < kv->value_len || _key_len > *key_len) {
                // TODO: We may want to indicate this differently
                res = -3;
                break;
            }

            // Read and copy the blob over
            fdb_blob_make(&blob, data, *data_len);
            fdb_kv_to_blob(kv, &blob);
            *data_len = fdb_blob_read((fdb_db_t)&ctx->kvdb, &blob);
            if (*data_len == 0) {
                res = -4;
                break;
            }

            // Copy the key over
            memcpy(key, _key, _key_len);
            *key_len = _key_len;
            res = 0;
            break;
        }
    }

    mutex_unlock(&ctx->service_lock);
    return res;
}

static int _iterator_init_callback(const void *context,
                                   store_service_iterator_t *iterator,
                                   const store_service_query_t *query)
{
    assert(context != NULL);
    assert(iterator != NULL);
    assert(query != NULL);
    assert(query->key != NULL);

    flashdb_store_service_iterator_t *flashdb_iterator =
        (flashdb_store_service_iterator_t *)iterator;
    memcpy(&flashdb_iterator->query, query, sizeof(store_service_query_t));

    flashdb_store_service_ctx_t *ctx = (flashdb_store_service_ctx_t *)context;
    fdb_kv_iterator_init(&ctx->kvdb, &flashdb_iterator->iterator);
    return 0;
}

static size_t _iterator_size_callback(const void *context)
{
    assert(context != NULL);

    return sizeof(flashdb_store_service_iterator_t);
}

const store_service_interface_t flashdb_store_service_interface = {
    .put = _put_callback,
    .get = _get_callback,
    .delete = _delete_callback,
    .iterator_size = _iterator_size_callback,
    .iterator_init = _iterator_init_callback,
    .iterator_next = _iterate_next_callback
};
