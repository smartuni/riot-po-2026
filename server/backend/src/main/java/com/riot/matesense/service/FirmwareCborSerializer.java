package com.riot.matesense.service;

import java.io.ByteArrayOutputStream;

final class FirmwareCborSerializer {

    private FirmwareCborSerializer() {}

    static byte[] serialize(GateCommandRecord r, byte[] signature) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        int arraySize = signature == null ? 9 : 10;
        writeArrayHeader(out, arraySize);

        writeSimple(out, r.version());
        writeSimple(out, r.messageType());
        writeSimple(out, r.recordType());

        writeBytes(out, r.writerId());
        writeUInt(out, r.sequence());
        writeUInt(out, r.hlcPhysical());
        writeUInt(out, r.hlcLogical());

        writeBytes(out, r.targetGateId());
        writeSimple(out, r.targetState());

        if (signature != null) {
            writeBytes(out, signature);
        }

        return out.toByteArray();
    }

    private static void writeArrayHeader(ByteArrayOutputStream out, int length) {
        writeTypeAndLength(out, 4, length);
    }

    private static void writeBytes(ByteArrayOutputStream out, byte[] bytes) {
        if (bytes == null) {
            throw new IllegalArgumentException("CBOR byte string must not be null");
        }
        writeTypeAndLength(out, 2, bytes.length);
        out.write(bytes, 0, bytes.length);
    }

    private static void writeUInt(ByteArrayOutputStream out, long value) {
        if (value < 0) {
            throw new IllegalArgumentException("CBOR unsigned integer cannot be negative: " + value);
        }
        writeTypeAndLength(out, 0, value);
    }

    private static void writeSimple(ByteArrayOutputStream out, int value) {
        if (value < 0 || value > 255 || (value >= 24 && value <= 31)) {
            throw new IllegalArgumentException("Invalid CBOR simple value: " + value);
        }
        if (value < 24) {
            out.write(0xE0 | value);
        } else {
            out.write(0xF8);
            out.write(value);
        }
    }

    private static void writeTypeAndLength(ByteArrayOutputStream out, int majorType, long value) {
        int prefix = majorType << 5;

        if (value < 24) {
            out.write(prefix | (int) value);
        } else if (value <= 0xFFL) {
            out.write(prefix | 24);
            out.write((int) value);
        } else if (value <= 0xFFFFL) {
            out.write(prefix | 25);
            writeU16(out, (int) value);
        } else if (value <= 0xFFFF_FFFFL) {
            out.write(prefix | 26);
            writeU32(out, value);
        } else {
            out.write(prefix | 27);
            writeU64(out, value);
        }
    }

    private static void writeU16(ByteArrayOutputStream out, int value) {
        out.write((value >>> 8) & 0xFF);
        out.write(value & 0xFF);
    }

    private static void writeU32(ByteArrayOutputStream out, long value) {
        out.write((int) (value >>> 24) & 0xFF);
        out.write((int) (value >>> 16) & 0xFF);
        out.write((int) (value >>> 8) & 0xFF);
        out.write((int) value & 0xFF);
    }

    private static void writeU64(ByteArrayOutputStream out, long value) {
        for (int shift = 56; shift >= 0; shift -= 8) {
            out.write((int) (value >>> shift) & 0xFF);
        }
    }
}
