#ifndef COSE_SERVICE_H
#define COSE_SERVICE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cose/sign.h"

// Maximale KID-Größe in Bytes
#define MAX_KEY_ID_SIZE 32

// Allgemeine Erfolg und Fehlercodes für Signatur-Operationen
#define COSE_SIGN_SUCCESS               0
#define COSE_SIGN_ERR_INVALID_ARGS    -100
#define COSE_SIGN_ERR_ENCODE_FAILED   -101

// Fehlercodes für Verify-Funktion
#define COSE_VERIFY_SUCCESS                   0
#define COSE_VERIFY_ERR_DECODE_FAILED       -200
#define COSE_VERIFY_ERR_SIGNATURE_ITER_FAIL -201
#define COSE_VERIFY_ERR_KID_UNKNOWN         -202
#define COSE_VERIFY_ERR_VERIFY_FAIL         -203
#define COSE_VERIFY_ERR_KID_EXTRACTION_FAIL -204
#define COSE_VERIFY_ERR_BUFFER_TOO_SMALL    -205

// Fehlercodes für KID-Extraktion (extract_kid_from_unprotected)
#define KID_OK                             0
#define KID_ERR_CBOR_PARSER_INIT          -1
#define KID_ERR_NOT_ARRAY                 -2
#define KID_ERR_ENTER_ARRAY_FAILED        -3
#define KID_ERR_ADVANCE_TO_UNPROT         -4
#define KID_ERR_UNPROT_NOT_MAP            -5
#define KID_ERR_ENTER_MAP_FAILED          -6
#define KID_ERR_INVALID_KEY               -7
#define KID_ERR_ADVANCE_TO_VALUE          -8
#define KID_ERR_COPY_KID_FAILED           -9
#define KID_ERR_ADVANCE_MAP_NEXT         -10
#define KID_ERR_KID_NOT_FOUND            -11

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Signiert einen Payload mit Ed25519 und erzeugt eine CBOR-codierte COSE_Sign1-Struktur.
 *
 * @param payload         Pointer auf den zu signierenden Binärdaten-Buffer
 * @param payload_len     Länge des Payloads in Bytes
 * @param encoding_buf    Interner Arbeits-/Zwischenpuffer zur Kodierung (vom Aufrufer bereitgestellt)
 *                        Muss ausreichend groß sein, z. B. 2048 Bytes.
 * @param out_buf_ptr     Pointer auf den Ausgabepuffer (wird intern gesetzt; zeigt auf encoding_buf oder intern allokiert)
 * @param out_len         Ausgabelänge des Puffers nach erfolgreicher Kodierung
 * @return                COSE_SIGN_SUCCESS bei Erfolg, sonst Fehlercode (z. B. COSE_SIGN_ERR_*)
 */
int sign_payload(const uint8_t *payload, size_t payload_len, uint8_t *encoding_buf,
                 uint8_t **out_buf_ptr, size_t *out_len);


/**
 * Verifiziert und dekodiert eine COSE_Sign1-Nachricht.
 *
 * @param encodedMsg     Eingabepuffer mit COSE_Sign1-Daten
 * @param len            Länge des Eingabepuffers
 * @param outbuf         Ausgabepuffer für den extrahierten Payload
 * @param outbuf_len     Größe des Ausgabepuffers
 * @param payload_len    Tatsächliche Länge des extrahierten Payloads
 * @return               COSE_VERIFY_SUCCESS bei Erfolg, sonst Fehlercode (COSE_VERIFY_ERR_*)
 */
int verify_decode(uint8_t *encodedMsg, size_t len,
                  uint8_t *outbuf, size_t outbuf_len, size_t *payload_len);


/**
 * Extrahiert das KID (Key ID) aus dem ungeschützten Header einer COSE_Sign1-Signatur.
 *
 * @param sig         Pointer auf COSE-Signatur-Decoder-Struktur
 * @param kid_buf     Zielpuffer für das extrahierte KID (max. MAX_KEY_ID_SIZE Bytes)
 * @param kid_len     Ein-/Ausgabeparameter: Eingabe = Größe von kid_buf, Ausgabe = Länge des extrahierten KID
 * @return            KID_OK bei Erfolg, sonst Fehlercode (z. B. KID_ERR_*)
 */
int extract_kid_from_unprotected(cose_signature_dec_t *sig,
                                 uint8_t *kid_buf, size_t *kid_len);

#ifdef __cplusplus
}
#endif

#endif // COSE_SERVICE_H