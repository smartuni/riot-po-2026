#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup Crypto service interface
 * @{
 */
/**
 * @brief Function type for signing data
 *
 * @param context       Context pointer provided during service initialization
 * @param kid           Key ID to use in the signature
 * @param kid_len       Length of @p kid in bytes
 * @param data          Pointer to the data to be signed
 * @param data_len      Length of the data in bytes
 * @param signature     Pointer to the buffer to store the signature
 * @param signature_len Pointer to the length of the signature buffer; updated
 *                     with the actual signature length upon success
 *
 * @retval 0 on success
 * @retval negative value on error
 */
typedef int (*crypto_service_sign_t)(const void *context, const uint8_t *kid,
                                     size_t kid_len, const void *data, size_t data_len,
                                     void *signature, size_t *signature_len);

/**
 * @brief Function type for verifying signatures
 *
 * @param context       Context pointer provided during service initialization
 * @param kid           Key ID expected in the signature
 * @param kid_len       Length of @p kid in bytes
 * @param data          Pointer to the signed data
 * @param data_len      Length of the signed data in bytes
 * @param signature     Pointer to the signature to be verified
 * @param signature_len Length of the signature in bytes
 *
 * @retval 0 on successful verification
 * @retval negative value on error or verification failure
 */
typedef int (*crypto_service_verify_t)(const void *context, const uint8_t *kid,
                                       size_t kid_len, const void *data, size_t data_len,
                                       const void *signature, size_t signature_len);

/**
 * @brief Interface to the table crypto service
 */
typedef struct {
    /**< Function for signing records */
    crypto_service_sign_t sign;

    /**< Function for verifying record signatures */
    crypto_service_verify_t verify;
} crypto_service_interface_t;

/**
 * @brief Table crypto service
 */
typedef struct {
    /**< Interface to the table crypto service */
    const crypto_service_interface_t *interface;

    /**< Context for the table crypto service */
    const void *context;
} crypto_service_t;
/** @} */

/**
 * @brief Sign raw data using the crypto service
 *
 * @param ctx               Pointer to the service
 * @param signer_id         Pointer to the ID of the node that signs
 * @param data              Pointer to the data to sign
 * @param data_len          Length of @p data
 * @param signature         Pointer of the buffer to store the signature. If NULL, only
 *                          the space is calculated.
 * @param signature_len     Pointer to store the length of the signature. If @p signature
 *                          is not NULL, it should contain its length.
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int crypto_service_sign(crypto_service_t *service, const uint8_t *kid, size_t kid_len,
                        const void *data, size_t data_len, void *signature,
                        size_t *signature_len);

/**
 * @brief Verify a signature on raw data using the crypto service
 *
 * @param ctx               Pointer to the service
 * @param signer_id         Pointer to the ID of the node that signs
 * @param data              Pointer to the data to verify
 * @param data_len          Length of @p data
 * @param signature         Pointer to the signature to verify
 * @param signature_len     Length of @p signature
 *
 * @retval 0 on success
 * @retval negative value on error
 */
int crypto_service_verify(crypto_service_t *service, const uint8_t *kid, size_t kid_len,
                          const void *data, size_t data_len, const void *signature,
                          size_t signature_len);
