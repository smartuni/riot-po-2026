package com.riot.matesense.service;

import org.junit.jupiter.api.Test;

import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertThrows;

class FirmwareCborSerializerTest {

    private static final HexFormat HEX = HexFormat.of();
    private static final byte[] WRITER_ID = HEX.parseHex("12121212");
    private static final byte[] GATE_ID_1 = HEX.parseHex("00000001");
    private static final byte[] TEST_SIGNATURE = new byte[16];

    @Test
    void serializesSimpleValueUnder24() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 0), null);
        assertThat(result[0] & 0xFF).isEqualTo(0x89);
        assertThat(result[1] & 0xFF).isEqualTo(0xE1);
        assertThat(result[2] & 0xFF).isEqualTo(0xE1);
        assertThat(result[3] & 0xFF).isEqualTo(0xE3);
    }

    @Test
    void serializesByteStringCorrectly() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 0), null);
        int writerIdOffset = 4;
        assertThat(result[writerIdOffset] & 0xFF).isEqualTo(0x44);
        assertThat(result[writerIdOffset + 1]).isEqualTo((byte) 0x12);
        assertThat(result[writerIdOffset + 2]).isEqualTo((byte) 0x12);
        assertThat(result[writerIdOffset + 3]).isEqualTo((byte) 0x12);
        assertThat(result[writerIdOffset + 4]).isEqualTo((byte) 0x12);
    }

    @Test
    void serializesSmallUnsignedIntegerAsDirectByte() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 0), null);
        int sequenceOffset = 9;
        assertThat(result[sequenceOffset] & 0xFF).isEqualTo(1);
        int hlcLogicalOffset = sequenceOffset + 1 + 1;
        assertThat(result[hlcLogicalOffset] & 0xFF).isEqualTo(0);
    }

    @Test
    void serializesSignatureWhenProvided() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 1), TEST_SIGNATURE);
        assertThat(result[0] & 0xFF).isEqualTo(0x8A);
        int lastElement = result[result.length - 1];
        assertThat(lastElement).isEqualTo(TEST_SIGNATURE[TEST_SIGNATURE.length - 1]);
    }

    @Test
    void unsignedAndSignedDifferOnlyBySignatureArraySize() {
        byte[] unsigned = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 0), null);
        byte[] signed = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1, 1, 0, GATE_ID_1, 0), TEST_SIGNATURE);
        assertThat(unsigned[0] & 0xFF).isEqualTo(0x89);
        assertThat(signed[0] & 0xFF).isEqualTo(0x8A);
        assertThat(unsigned.length + 1 + TEST_SIGNATURE.length).isEqualTo(signed.length);
    }

    @Test
    void serializesUnsignedIntegerInRange24to255() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 100, 100, 0, GATE_ID_1, 0), null);
        int seqOffset = 9;
        assertThat(result[seqOffset] & 0xFF).isEqualTo(0x18);
        assertThat(result[seqOffset + 1] & 0xFF).isEqualTo(100);
    }

    @Test
    void serializesUnsignedIntegerInRange256to65535() {
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, 1000, 1000, 0, GATE_ID_1, 0), null);
        int seqOffset = 9;
        assertThat(result[seqOffset] & 0xFF).isEqualTo(0x19);
        assertThat(result[seqOffset + 1] & 0xFF).isEqualTo((1000 >> 8) & 0xFF);
        assertThat(result[seqOffset + 2] & 0xFF).isEqualTo(1000 & 0xFF);
    }

    @Test
    void rejectsNullByteString() {
        assertThrows(IllegalArgumentException.class,
                () -> FirmwareCborSerializer.serialize(
                        new GateCommandRecord(1, 1, 3, null, 1, 1, 0, GATE_ID_1, 0), null));
    }

    @Test
    void rejectsNegativeUnsignedInteger() {
        assertThrows(IllegalArgumentException.class,
                () -> FirmwareCborSerializer.serialize(
                        new GateCommandRecord(1, 1, 3, WRITER_ID, -1, 1, 0, GATE_ID_1, 0), null));
    }

    @Test
    void outputIsValidCbor() {
        long epochSeconds = 1_760_000_000L;
        byte[] result = FirmwareCborSerializer.serialize(
                new GateCommandRecord(1, 1, 3, WRITER_ID, epochSeconds, epochSeconds, 0, GATE_ID_1, 1),
                TEST_SIGNATURE);
        assertThat(result[0] & 0xFF).isEqualTo(0x8A);
        assertThat(result.length).isGreaterThan(20);
    }
}
