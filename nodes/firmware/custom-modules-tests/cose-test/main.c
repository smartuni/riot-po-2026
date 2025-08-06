#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ztimer.h"

#include "cose.h"
#include "cose/sign.h"
#include "cose/crypto.h"
#include "embUnit.h"
#include "memarray.h"
#include "cose-service.h"

#include "benchmark.h"

int main(void)
{
    ztimer_sleep(ZTIMER_SEC, 3);

    const uint8_t payload[] = "Input string";

    // --- Encode code ---
    uint8_t *encoded_ptr = NULL;
    uint8_t encode_outbuf[256];
    size_t encoded_len = 0;
    BENCHMARK_FUNC("Signieren:", 5, sign_payload(payload, sizeof(payload),encode_outbuf,&encoded_ptr, &encoded_len));

    // --- Verify code ---
    uint8_t verify_outbuf[256];  // ausreichend groß dimensionieren
    size_t verify_payload_len = 0;

    BENCHMARK_FUNC("Validieren:", 5,verify_decode(encoded_ptr, encoded_len,verify_outbuf, sizeof(verify_outbuf),&verify_payload_len));

    // --- Auswertung ---
    int sign_result = sign_payload(payload, sizeof(payload),encode_outbuf,&encoded_ptr, &encoded_len);
    if (sign_result != 0) {
        printf("❌ Signing fehlgeschlagen mit Fehlercode: %d\n", sign_result);
    }

    int verify_result = verify_decode(encoded_ptr, encoded_len,
        verify_outbuf, sizeof(verify_outbuf),
        &verify_payload_len);
    if (verify_result != 0) {
        printf("❌ Verifikation fehlgeschlagen mit Fehlercode: %d\n", verify_result);
    }
    else {
        printf("✅ Verifikation erfolgreich\n");
        printf("🔍 Dekodierter Payload (%i Bytes):\n", verify_payload_len);
        printf("\"%.*s\"\n", (int)verify_payload_len, verify_outbuf);

        if (verify_payload_len != sizeof(payload)) {
            printf("❌ Fehlgeschlagen: Länge der Payloads stimmt nicht überein\n");
        }
        else if (memcmp(payload, verify_outbuf, verify_payload_len) != 0) {
            printf("❌ Fehlgeschlagen: Inhalt der Payloads stimmt nicht überein\n");
        }
        else {
            printf("✅ Erfolg: Signaturprüfung und Payloadvergleich erfolgreich\n");
        }
    }

    return 0;
}
