package com.riot.matesense.service;

import java.io.ByteArrayOutputStream;

final class CoseSign1Encoder {

    private CoseSign1Encoder() {}

    static byte[] encode(byte[] kid, byte[] ed25519Signature) {
        if (kid == null || kid.length != 4) {
            throw new IllegalArgumentException("KID must be exactly 4 bytes");
        }
        if (ed25519Signature == null || ed25519Signature.length != 64) {
            throw new IllegalArgumentException("Ed25519 signature must be exactly 64 bytes");
        }

        ByteArrayOutputStream out = new ByteArrayOutputStream();

        // array(4) — COSE_Sign1
        writeArrayHeader(out, 4);

        // [0] protected: bstr_wrapped({1: -8, 4: h'KID'})
        out.writeBytes(buildProtectedHeader(kid));

        // [1] unprotected: {}
        out.write(0xA0);

        // [2] payload: nil (external data)
        out.write(0xF6);

        // [3] signature: bstr(64)
        writeBytes(out, ed25519Signature);

        return out.toByteArray();
    }

    private static byte[] buildProtectedHeader(byte[] kid) {
        ByteArrayOutputStream map = new ByteArrayOutputStream();

        // {1: -8, 4: h'kid'}
        map.write(0xA2);   // map(2)
        map.write(0x01);   // key 1 — alg
        map.write(0x27);   // -8 — EdDSA
        map.write(0x04);   // key 4 — kid
        writeBytes(map, kid);

        byte[] mapBytes = map.toByteArray();
        ByteArrayOutputStream bstr = new ByteArrayOutputStream();
        writeBytes(bstr, mapBytes);
        return bstr.toByteArray();
    }

    private static void writeArrayHeader(ByteArrayOutputStream out, int length) {
        writeTypeAndLength(out, 4, length);
    }

    private static void writeBytes(ByteArrayOutputStream out, byte[] bytes) {
        writeTypeAndLength(out, 2, bytes.length);
        out.write(bytes, 0, bytes.length);
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
            out.write((int) (value >>> 8));
            out.write((int) value);
        } else if (value <= 0xFFFF_FFFFL) {
            out.write(prefix | 26);
            out.write((int) (value >>> 24));
            out.write((int) (value >>> 16));
            out.write((int) (value >>> 8));
            out.write((int) value);
        }
    }
}
