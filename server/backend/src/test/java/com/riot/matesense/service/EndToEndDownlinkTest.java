package com.riot.matesense.service;

import org.junit.jupiter.api.Test;

import java.util.Base64;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Simulates the full end-to-end flow without LoRaWAN hardware:
 * Backend signs → CBOR over LoRaWAN → Firmware verifies.
 *
 * The firmware verification logic is reimplemented here in Java:
 * 1. Parse CBOR, extract record + signature
 * 2. Re-serialize unsigned CBOR (firmware's cbor_serialize_record_no_sig)
 * 3. Compute AES-CMAC over unsigned CBOR
 * 4. Constant-time compare with extracted signature
 */
class EndToEndDownlinkTest {

    private static final HexFormat HEX = HexFormat.of();
    private static final byte[] TEST_KEY = HEX.parseHex("2b7e151628aed2a6abf7158809cf4f3c");
    private static final CmacService cmacService = new CmacService();

    /**
     * Full round-trip: backend signs → firmware verifies.
     */
    @Test
    void backendSignsAndFirmwareVerifies() {
        // ── Backend side ──
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x12, 0x12, 0x12, 0x12},
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] cmac = cmacService.computeCmac(TEST_KEY, unsignedCbor);
        byte[] signature = java.util.Arrays.copyOf(cmac, 16);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, signature);

        assertThat(signedCbor.length).isLessThanOrEqualTo(51);

        // ── Over LoRaWAN: Base64-encode, then decode (simulate TTN) ──
        String base64Payload = Base64.getEncoder().encodeToString(signedCbor);
        byte[] receivedCbor = Base64.getDecoder().decode(base64Payload);

        assertThat(receivedCbor).isEqualTo(signedCbor);

        // ── Firmware side ──

        // Step 1: Extract version, message_type from CBOR (firmware validates these)
        assertThat(receivedCbor[0] & 0xFF).isEqualTo(0x8A); // array(10) with signature
        assertThat(receivedCbor[1] & 0xFF).isEqualTo(0xE1); // simple(1) version
        assertThat(receivedCbor[2] & 0xFF).isEqualTo(0xE1); // simple(1) message_type

        // Step 2: Extract record via cbor_deserialize (simulated byte manipulation)
        // The signature is the last CBOR element: 0x50 (bytes(16)) + 16 bytes
        byte[] extractedSig = new byte[16];
        System.arraycopy(receivedCbor, receivedCbor.length - 16, extractedSig, 0, 16);

        // Step 3: Re-serialize unsigned CBOR via cbor_serialize_record_no_sig
        byte[] reUnsignedCbor = new byte[receivedCbor.length - 17];
        reUnsignedCbor[0] = (byte) 0x89; // array(9), changed from array(10)
        System.arraycopy(receivedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        // Step 4: Compute CMAC via aes128_cmac (firmware)
        byte[] computedTag = cmacService.computeCmac(TEST_KEY, reUnsignedCbor);

        // Step 5: Constant-time compare (firmware's _constant_time_memcmp)
        int diff = 0;
        for (int i = 0; i < 16; i++) {
            diff |= computedTag[i] ^ extractedSig[i];
        }
        assertThat(diff).isEqualTo(0);
    }

    /**
     * Tampered payload (single bit flip) is rejected.
     */
    @Test
    void tamperedPayloadIsRejected() {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x12, 0x12, 0x12, 0x12},
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] cmac = cmacService.computeCmac(TEST_KEY, unsignedCbor);
        byte[] signature = java.util.Arrays.copyOf(cmac, 16);
        byte[] originalSignedCbor = FirmwareCborSerializer.serialize(record, signature);

        // Flip one bit in the gate_num field (byte at index 18)
        byte[] tamperedCbor = originalSignedCbor.clone();
        tamperedCbor[18] ^= 0x01;

        // Firmware extracts unsigned CBOR from tampered payload
        byte[] extractedSig = new byte[16];
        System.arraycopy(tamperedCbor, tamperedCbor.length - 16, extractedSig, 0, 16);

        byte[] reUnsignedCbor = new byte[tamperedCbor.length - 17];
        reUnsignedCbor[0] = (byte) 0x89;
        System.arraycopy(tamperedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        byte[] computedTag = cmacService.computeCmac(TEST_KEY, reUnsignedCbor);

        // CMAC mismatch → firmware drops silently
        assertThat(HEX.formatHex(computedTag)).isNotEqualTo(HEX.formatHex(extractedSig));
    }

    /**
     * Wrong key → CMAC mismatch.
     */
    @Test
    void wrongKeyIsRejected() {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x12, 0x12, 0x12, 0x12},
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] cmac = cmacService.computeCmac(TEST_KEY, unsignedCbor);
        byte[] signature = java.util.Arrays.copyOf(cmac, 16);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, signature);

        // Firmware uses a different key (all zeros)
        byte[] wrongKey = new byte[16];
        byte[] extractedSig = new byte[16];
        System.arraycopy(signedCbor, signedCbor.length - 16, extractedSig, 0, 16);

        byte[] reUnsignedCbor = new byte[signedCbor.length - 17];
        reUnsignedCbor[0] = (byte) 0x89;
        System.arraycopy(signedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        byte[] computedTag = cmacService.computeCmac(wrongKey, reUnsignedCbor);

        // Mismatch → rejected
        assertThat(HEX.formatHex(computedTag)).isNotEqualTo(HEX.formatHex(extractedSig));
    }

    /**
     * Signed CBOR matches firmware's expected wire format (simple values,
     * byte strings, proper array structure).
     */
    @Test
    void cborWireFormatMatchesFirmwareExpectations() {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x01, 0x02, 0x03, 0x04},
                0xDACB, 0xDACB, 0x0A,
                new byte[]{0x11, 0x12, 0x13, 0x14},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        // Use a 16-byte signature like real CMAC
        byte[] signature = HEX.parseHex("0102030405060708090a0b0c0d0e0f10");
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, signature);

        // Check array header
        assertThat(unsignedCbor[0] & 0xFF).isEqualTo(0x89); // array(9)
        assertThat(signedCbor[0] & 0xFF).isEqualTo(0x8A);   // array(10)

        // Check simple values: version(1), message_type(1), record_type(3)
        assertThat(unsignedCbor[1] & 0xFF).isEqualTo(0xE1);
        assertThat(unsignedCbor[2] & 0xFF).isEqualTo(0xE1);
        assertThat(unsignedCbor[3] & 0xFF).isEqualTo(0xE3);

        // Check writer ID byte string: 0x44 (bytes(4)) + 4 bytes
        assertThat(unsignedCbor[4] & 0xFF).isEqualTo(0x44);
        assertThat(unsignedCbor[5]).isEqualTo((byte) 0x01);
        assertThat(unsignedCbor[6]).isEqualTo((byte) 0x02);
        assertThat(unsignedCbor[7]).isEqualTo((byte) 0x03);
        assertThat(unsignedCbor[8]).isEqualTo((byte) 0x04);

        // Check sequence: uint(0xDACB) → 0x19 + 2 bytes
        assertThat(unsignedCbor[9] & 0xFF).isEqualTo(0x19);
        assertThat(unsignedCbor[10] & 0xFF).isEqualTo(0xDA);
        assertThat(unsignedCbor[11] & 0xFF).isEqualTo(0xCB);

        // Check HLC physical: uint(0xDACB) → 0x19 + 2 bytes
        assertThat(unsignedCbor[12] & 0xFF).isEqualTo(0x19);
        assertThat(unsignedCbor[13] & 0xFF).isEqualTo(0xDA);
        assertThat(unsignedCbor[14] & 0xFF).isEqualTo(0xCB);

        // Check HLC logical: uint(0x0A) → 0x0A
        assertThat(unsignedCbor[15] & 0xFF).isEqualTo(0x0A);

        // Check gate ID byte string: 0x44 + 4 bytes
        assertThat(unsignedCbor[16] & 0xFF).isEqualTo(0x44);
        assertThat(unsignedCbor[17]).isEqualTo((byte) 0x11);
        assertThat(unsignedCbor[18]).isEqualTo((byte) 0x12);
        assertThat(unsignedCbor[19]).isEqualTo((byte) 0x13);
        assertThat(unsignedCbor[20]).isEqualTo((byte) 0x14);

        // Check gate state: simple(1) → 0xE1
        assertThat(unsignedCbor[21] & 0xFF).isEqualTo(0xE1);

        // Check signature in signed CBOR: 0x50 (bytes(16)) + 16 bytes
        assertThat(signedCbor[22] & 0xFF).isEqualTo(0x50);
        for (int i = 0; i < 16; i++) {
            assertThat(signedCbor[23 + i]).isEqualTo(signature[i]);
        }

        // Unsigned length = signed length - 17 (1 byte header + 16 sig bytes)
        assertThat(unsignedCbor.length).isEqualTo(signedCbor.length - 17);

        // Both fit within 51-byte limit
        assertThat(unsignedCbor.length).isLessThanOrEqualTo(51);
        assertThat(signedCbor.length).isLessThanOrEqualTo(51);
    }

    /**
     * Prints the exact hex payload that would be sent over LoRaWAN.
     * Copy hex from test output to verify against firmware.
     */
    @Test
    void printExactDownlinkHex() {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x12, 0x12, 0x12, 0x12},
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] cmac = cmacService.computeCmac(TEST_KEY, unsignedCbor);
        byte[] signature = java.util.Arrays.copyOf(cmac, 16);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, signature);

        System.out.println("=== Downlink Payload (hex, " + signedCbor.length + " bytes) ===");
        System.out.println(HEX.formatHex(signedCbor));
        System.out.println("=== Base64 for TTN ===");
        System.out.println(Base64.getEncoder().encodeToString(signedCbor));
        System.out.println("=== Unsigned CBOR (hex, " + unsignedCbor.length + " bytes) ===");
        System.out.println(HEX.formatHex(unsignedCbor));
        System.out.println("=== CMAC tag (hex) ===");
        System.out.println(HEX.formatHex(signature));

        assertThat(signedCbor.length).isLessThanOrEqualTo(51);
    }
}
