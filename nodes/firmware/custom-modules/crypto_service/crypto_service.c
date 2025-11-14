#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "crypto_service.h"

int crypto_service_sign(crypto_service_t *service, const uint8_t *kid, size_t kid_len,
                        const void *data, size_t data_len, void *signature,
                        size_t *signature_len)
{
    assert(service != NULL);
    assert(kid != NULL);
    assert(data != NULL);
    assert(signature_len != NULL);

    return service->interface.sign(service->context, kid, kid_len, data, data_len,
                                   signature, signature_len);
}

int crypto_service_verify(crypto_service_t *service, const uint8_t *kid, size_t kid_len,
                          const void *data, size_t data_len, const void *signature,
                          size_t signature_len)
{
    assert(service != NULL);
    assert(kid != NULL);
    assert(data != NULL);
    assert(signature != NULL);

    return service->interface.verify(service->context, kid, kid_len, data, data_len,
                                     signature, signature_len);
}
