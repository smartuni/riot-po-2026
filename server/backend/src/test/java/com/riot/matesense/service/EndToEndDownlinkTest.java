package com.riot.matesense.service;

import org.bouncycastle.crypto.params.Ed25519PublicKeyParameters;
import org.bouncycastle.crypto.signers.Ed25519Signer;
import org.junit.jupiter.api.Test;

import java.util.Base64;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Simulates the full end-to-end flow without LoRaWAN hardware:
 * Backend signs with Ed25519 COSE → CBOR over LoRaWAN → Firmware verifies.
 *
 * The firmware verification is simulated here:
 * 1. Parse CBOR, extract COSE Sign1 signature
 * 2. Re-serialize unsigned CBOR (firmware's cbor_serialize_record_no_sig)
 * 3. Parse COSE Sign1, extract KID and raw Ed25519 signature
 * 4. Rebuild Sig_structure (firmware's libcose does this internally)
 * 5. Verify Ed25519 over the Sig_structure via Bouncy Castle
 */
class EndToEndDownlinkTest {

    private static final HexFormat HEX = HexFormat.of();

    private static final byte[] BACKEND_SEED = HEX.parseHex(
            "ce65cd03fc31cc7137f193e4e0696cf31a15a3507959f5eebdaa849a8cbb7c9d");
    private static final byte[] BACKEND_PUBLIC_KEY = HEX.parseHex(
            "24ccc1fa01cb1e92d541cbac95e6f9e52c16a874b01a29e59aa9c6da824b6248");
    private static final byte[] WRONG_PUBLIC_KEY = HEX.parseHex(
            "98390187359cad019ba905660ff2ac5df1d21cd313ed75ada78b9f42dbbe9e5e");
    private static final byte[] BACKEND_KID = HEX.parseHex("12121212");

    @Test
    void backendSignsAndFirmwareVerifies() {
        // ── Backend side (using new COSE-correct signing) ──
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x12, 0x12, 0x12, 0x12},
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01},
                1
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);

        // Build COSE Sign1 properly: signs Sig_structure, not raw payload
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // ── Over LoRaWAN: Base64-encode, then decode (simulate TTN) ──
        String base64Payload = Base64.getEncoder().encodeToString(signedCbor);
        byte[] receivedCbor = Base64.getDecoder().decode(base64Payload);

        assertThat(receivedCbor).isEqualTo(signedCbor);

        // ── Firmware side (simulated) ──

        // Step 1: Validate CBOR array(10) for signed payload
        assertThat(receivedCbor[0] & 0xFF).isEqualTo(0x8A); // array(10)

        // Step 2: Extract COSE signature from end of CBOR
        // CBOR encodes the signature as a byte string at the end
        int coseSigLen = coseSignature.length;
        int bstrHdrLen = coseSigLen >= 24 ? 2 : 1;

        byte[] extractedCoseSig = new byte[coseSigLen];
        System.arraycopy(receivedCbor, receivedCbor.length - coseSigLen,
                extractedCoseSig, 0, coseSigLen);

        // Verify the byte string header
        int bstrHdrByte = receivedCbor[receivedCbor.length - coseSigLen - 1] & 0xFF;
        assertThat(bstrHdrByte & 0xE0).isEqualTo(0x40); // major type 2 (bstr)

        // Step 3: Parse COSE Sign1 structure
        // COSE_Sign1 = [protected_bstr, {}, nil, sig_bstr(64)]
        assertThat(extractedCoseSig[0] & 0xFF).isEqualTo(0x84); // array(4)

        // Extract raw signature (last 64 bytes of COSE)
        byte[] extractedRawSig = new byte[64];
        System.arraycopy(extractedCoseSig, extractedCoseSig.length - 64,
                extractedRawSig, 0, 64);

        // Step 4: Verify KID in protected header
        // protected header: bstr_wrapped({1: -8, 4: h'12121212'})
        // Skip array(4) header (1 byte), then bstr header
        int pos = 1;
        int bstrLen = extractedCoseSig[pos] & 0x1F;
        pos += (bstrLen >= 24) ? 2 : 1;
        // Inside bstr: map(2), key 1, -8, key 4, bstr(4), KID bytes
        pos += 3; // map(2) + key 1 + 0x27(-8)
        assertThat(extractedCoseSig[pos] & 0xFF).isEqualTo(0x04); // key 4
        pos++;
        assertThat(extractedCoseSig[pos] & 0xE0).isEqualTo(0x40); // bstr
        pos++;
        byte[] extractedKid = new byte[4];
        System.arraycopy(extractedCoseSig, pos, extractedKid, 0, 4);
        assertThat(HEX.formatHex(extractedKid)).isEqualTo(HEX.formatHex(BACKEND_KID));

        // Step 5: Re-serialize unsigned CBOR (firmware's cbor_serialize_record_no_sig)
        byte[] reUnsignedCbor = new byte[signedCbor.length - coseSigLen - bstrHdrLen];
        reUnsignedCbor[0] = (byte) 0x89; // array(9) — unsigned
        System.arraycopy(signedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        // Step 6: Rebuild the COSE Sig_structure (firmware's libcose does this internally)
        // Sig_structure = ["Signature1", protected, external_aad, payload]
        byte[] protectedHeaders = CoseSign1Encoder.buildProtectedHeader(BACKEND_KID);
        byte[] sigStructure = CoseSign1Encoder.buildSigStructure(protectedHeaders, reUnsignedCbor);

        // Step 7: Verify Ed25519 signature over the Sig_structure (matches libcose behavior)
        Ed25519PublicKeyParameters pubKey = new Ed25519PublicKeyParameters(BACKEND_PUBLIC_KEY, 0);
        Ed25519Signer verifier = new Ed25519Signer();
        verifier.init(false, pubKey);
        verifier.update(sigStructure, 0, sigStructure.length);
        assertThat(verifier.verifySignature(extractedRawSig)).isTrue();
    }

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
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] originalSignedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Flip one bit in the gate_num field
        byte[] tamperedCbor = originalSignedCbor.clone();
        tamperedCbor[18] ^= 0x01;

        // Extract as firmware would
        int coseSigLen = coseSignature.length;
        int bstrHdrLen = coseSigLen >= 24 ? 2 : 1;

        byte[] extractedCoseSig = new byte[coseSigLen];
        System.arraycopy(tamperedCbor, tamperedCbor.length - coseSigLen,
                extractedCoseSig, 0, coseSigLen);

        byte[] reUnsignedCbor = new byte[tamperedCbor.length - coseSigLen - bstrHdrLen];
        reUnsignedCbor[0] = (byte) 0x89;
        System.arraycopy(tamperedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        byte[] extractedRawSig = new byte[64];
        System.arraycopy(extractedCoseSig, extractedCoseSig.length - 64,
                extractedRawSig, 0, 64);

        // Rebuild Sig_structure (as libcose does internally)
        byte[] protectedHeaders = CoseSign1Encoder.buildProtectedHeader(BACKEND_KID);
        byte[] sigStructure = CoseSign1Encoder.buildSigStructure(protectedHeaders, reUnsignedCbor);

        Ed25519PublicKeyParameters pubKey = new Ed25519PublicKeyParameters(BACKEND_PUBLIC_KEY, 0);
        Ed25519Signer verifier = new Ed25519Signer();
        verifier.init(false, pubKey);
        verifier.update(sigStructure, 0, sigStructure.length);
        assertThat(verifier.verifySignature(extractedRawSig)).isFalse();
    }

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
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Use a different valid key pair
        byte[] wrongPubKey = WRONG_PUBLIC_KEY;

        int coseSigLen = coseSignature.length;
        int bstrHdrLen = coseSigLen >= 24 ? 2 : 1;

        byte[] extractedCoseSig = new byte[coseSigLen];
        System.arraycopy(signedCbor, signedCbor.length - coseSigLen,
                extractedCoseSig, 0, coseSigLen);

        byte[] reUnsignedCbor = new byte[signedCbor.length - coseSigLen - bstrHdrLen];
        reUnsignedCbor[0] = (byte) 0x89;
        System.arraycopy(signedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);

        byte[] extractedRawSig = new byte[64];
        System.arraycopy(extractedCoseSig, extractedCoseSig.length - 64,
                extractedRawSig, 0, 64);

        // Rebuild Sig_structure (as libcose does internally)
        byte[] protectedHeaders = CoseSign1Encoder.buildProtectedHeader(BACKEND_KID);
        byte[] sigStructure = CoseSign1Encoder.buildSigStructure(protectedHeaders, reUnsignedCbor);

        Ed25519PublicKeyParameters wrongPub = new Ed25519PublicKeyParameters(wrongPubKey, 0);
        Ed25519Signer verifier = new Ed25519Signer();
        verifier.init(false, wrongPub);
        verifier.update(sigStructure, 0, sigStructure.length);
        assertThat(verifier.verifySignature(extractedRawSig)).isFalse();
    }

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
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Check array headers
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

        // Check signature in signed CBOR: bstr of COSE length
        int coseLen = coseSignature.length;
        int bstrHdrOffset = unsignedCbor.length; // after unsigned CBOR elements
        int bstrHdrByte = signedCbor[bstrHdrOffset] & 0xFF;
        assertThat(bstrHdrByte & 0xE0).isEqualTo(0x40); // bstr major type
        assertThat(signedCbor.length).isEqualTo(unsignedCbor.length + (coseLen >= 24 ? 2 : 1) + coseLen);
    }

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
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        System.out.println("=== Downlink Payload (hex, " + signedCbor.length + " bytes) ===");
        System.out.println(HEX.formatHex(signedCbor));
        System.out.println("=== Base64 for TTN ===");
        System.out.println(Base64.getEncoder().encodeToString(signedCbor));
        System.out.println("=== Unsigned CBOR (hex, " + unsignedCbor.length + " bytes) ===");
        System.out.println(HEX.formatHex(unsignedCbor));
        System.out.println("=== COSE Sign1 (hex, " + coseSignature.length + " bytes) ===");
        System.out.println(HEX.formatHex(coseSignature));
    }
}
