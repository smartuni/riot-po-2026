package com.riot.matesense.service;

import org.bouncycastle.crypto.params.Ed25519PrivateKeyParameters;
import org.bouncycastle.crypto.signers.Ed25519Signer;

import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

final class CoseSign1Encoder {

    private CoseSign1Encoder() {}

    /**
     * Build a complete COSE_Sign1 message:
     *   1. Build the Sig_structure (["Signature1", protected, external_aad, payload])
     *   2. Sign the Sig_structure with Ed25519
     *   3. Wrap in COSE_Sign1 = [protected, {}, nil, signature]
     *
     * This matches libcose's cose_sign_encode with COSE_FLAGS_EXTDATA.
     */
    static byte[] buildCoseSign1(byte[] kid, byte[] unsignedCbor, byte[] signingKeySeed) {
        byte[] protectedHeaders = buildProtectedHeader(kid);
        byte[] sigStructure = buildSigStructure(protectedHeaders, unsignedCbor);
        byte[] rawSig = signEd25519(signingKeySeed, sigStructure);
        return encode(kid, rawSig);
    }

    /**
     * Build the COSE Sig_structure (RFC 8152 §4.3) for a single signer:
     *
     * Sig_structure = [
     *   "Signature1",          -- CBOR text string
     *   protected,             -- bstr of protected header map
     *   external_aad,          -- empty bstr
     *   payload                -- bstr of the actual signed payload
     * ]
     */
    static byte[] buildSigStructure(byte[] protectedHeaders, byte[] payload) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        writeArrayHeader(out, 4);
        writeTstr(out, "Signature1");
        out.writeBytes(protectedHeaders);
        writeBytes(out, new byte[0]);
        writeBytes(out, payload);
        return out.toByteArray();
    }

    /**
     * Build the protected header CBOR:
     *
     * bstr({1: -8, 4: h'KID'})
     *
     *   -- 1: -8   (alg = EdDSA)
     *   -- 4: KID  (kid as bstr)
     */
    static byte[] buildProtectedHeader(byte[] kid) {
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

    /**
     * Build a raw Ed25519 signature over the given data.
     */
    static byte[] signEd25519(byte[] signingKeySeed, byte[] data) {
        Ed25519PrivateKeyParameters privKey = new Ed25519PrivateKeyParameters(signingKeySeed, 0);
        Ed25519Signer signer = new Ed25519Signer();
        signer.init(true, privKey);
        signer.update(data, 0, data.length);
        return signer.generateSignature();
    }

    /**
     * Encode a raw Ed25519 signature into a COSE_Sign1 CBOR structure.
     *
     * COSE_Sign1 = [
     *   protected: bstr({1: -8, 4: h'KID'}),
     *   unprotected: {},
     *   payload: nil (external data),
     *   signature: bstr(Ed25519 sig)
     * ]
     */
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

    private static void writeArrayHeader(ByteArrayOutputStream out, int length) {
        writeTypeAndLength(out, 4, length);
    }

    private static void writeTstr(ByteArrayOutputStream out, String value) {
        byte[] utf8 = value.getBytes(StandardCharsets.UTF_8);
        writeTypeAndLength(out, 3, utf8.length);
        out.writeBytes(utf8);
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
