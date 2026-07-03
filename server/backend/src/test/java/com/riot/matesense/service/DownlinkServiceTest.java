package com.riot.matesense.service;

import org.junit.jupiter.api.Test;

import java.util.Base64;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertThrows;

class DownlinkServiceTest {

    private static final HexFormat HEX = HexFormat.of();

    private static final byte[] TEST_SIGNING_KEY_SEED = HEX.parseHex(
            "ce65cd03fc31cc7137f193e4e0696cf31a15a3507959f5eebdaa849a8cbb7c9d");
    private static final byte[] TEST_KID = HEX.parseHex("12121212");

    @Test
    void mapGateState_returnsCorrectValue() {
        assertThat(DownlinkService.mapGateState(0)).isEqualTo(0);
        assertThat(DownlinkService.mapGateState(1)).isEqualTo(1);
    }

    @Test
    void mapGateState_rejectsInvalidState() {
        assertThrows(IllegalArgumentException.class,
                () -> DownlinkService.mapGateState(-1));
        assertThrows(IllegalArgumentException.class,
                () -> DownlinkService.mapGateState(2));
    }

    @Test
    void buildSignedGateCommandCbor_returnsBase64() {
        String base64 = DownlinkService.buildSignedGateCommandCbor(1, 0, TEST_SIGNING_KEY_SEED, TEST_KID);
        assertThat(base64).isNotNull();
        assertThat(base64).isNotBlank();
        byte[] decoded = Base64.getDecoder().decode(base64);
        assertThat(decoded).isNotEmpty();
    }

    @Test
    void buildSignedGateCommandCbor_array10WithSignature() {
        String base64 = DownlinkService.buildSignedGateCommandCbor(1, 0, TEST_SIGNING_KEY_SEED, TEST_KID);
        byte[] decoded = Base64.getDecoder().decode(base64);
        assertThat(decoded[0] & 0xFF).isEqualTo(0x8A); // array(10)
    }

    @Test
    void buildSignedGateCommandCbor_containsCoseSignature() {
        String base64 = DownlinkService.buildSignedGateCommandCbor(1, 0, TEST_SIGNING_KEY_SEED, TEST_KID);
        byte[] decoded = Base64.getDecoder().decode(base64);

        // The last CBOR element is the signature byte string (bstr)
        // bstr of 79 bytes: 0x58 0x4F + 79 bytes
        int sigBstrHdrPos = decoded.length - 79 - 2;
        assertThat(decoded[sigBstrHdrPos] & 0xFF).isEqualTo(0x58);
        assertThat(decoded[sigBstrHdrPos + 1] & 0xFF).isEqualTo(0x4F);

        // Extract the COSE Sign1 and verify it starts with array(4) = 0x84
        byte[] cose = new byte[79];
        System.arraycopy(decoded, sigBstrHdrPos + 2, cose, 0, 79);
        assertThat(cose[0] & 0xFF).isEqualTo(0x84);
    }

    @Test
    void buildSignedGateCommandCbor_differentStatesProduceDifferentPayloads() {
        String p1 = DownlinkService.buildSignedGateCommandCbor(1, 0, TEST_SIGNING_KEY_SEED, TEST_KID);
        String p2 = DownlinkService.buildSignedGateCommandCbor(1, 1, TEST_SIGNING_KEY_SEED, TEST_KID);
        assertThat(p1).isNotEqualTo(p2);
    }

    @Test
    void buildSignedGateCommandCbor_differentGatesProduceDifferentPayloads() {
        String p1 = DownlinkService.buildSignedGateCommandCbor(1, 1, TEST_SIGNING_KEY_SEED, TEST_KID);
        String p2 = DownlinkService.buildSignedGateCommandCbor(2, 1, TEST_SIGNING_KEY_SEED, TEST_KID);
        assertThat(p1).isNotEqualTo(p2);
    }

    @Test
    void signEd25519_produces64ByteSignature() {
        byte[] data = new byte[]{0x01, 0x02, 0x03};
        byte[] sig = CoseSign1Encoder.signEd25519(TEST_SIGNING_KEY_SEED, data);
        assertThat(sig).hasSize(64);
    }

    @Test
    void signEd25519_differentInputsProduceDifferentSignatures() {
        byte[] data1 = new byte[]{0x01, 0x02, 0x03};
        byte[] data2 = new byte[]{0x01, 0x02, 0x04};
        byte[] sig1 = CoseSign1Encoder.signEd25519(TEST_SIGNING_KEY_SEED, data1);
        byte[] sig2 = CoseSign1Encoder.signEd25519(TEST_SIGNING_KEY_SEED, data2);
        assertThat(HEX.formatHex(sig1)).isNotEqualTo(HEX.formatHex(sig2));
    }
}
