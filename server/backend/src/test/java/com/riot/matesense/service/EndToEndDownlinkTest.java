package com.riot.matesense.service;

import COSE.KeyKeys;
import COSE.Message;
import COSE.MessageTag;
import COSE.OneKey;
import COSE.Sign1Message;
import com.upokecenter.cbor.CBORObject;
import org.junit.jupiter.api.Test;

import java.util.Base64;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Simulates the full end-to-end flow without LoRaWAN hardware:
 * Backend signs with Ed25519 COSE → CBOR over LoRaWAN → Firmware verifies.
 *
 * The firmware verification is simulated here:
 * 1. Parse CBOR, extract the embedded COSE Sign1 message
 * 2. Rebuild the unsigned CBOR payload when needed
 * 3. Validate the COSE message against the backend public key
 */
class EndToEndDownlinkTest {

    private static final HexFormat HEX = HexFormat.of();

    private static final byte[] BACKEND_SEED = HEX.parseHex(
            "ce65cd03fc31cc7137f193e4e0696cf31a15a3507959f5eebdaa849a8cbb7c9d");
    private static final byte[] BACKEND_PUBLIC_KEY = HEX.parseHex(
            "24ccc1fa01cb1e92d541cbac95e6f9e52c16a874b01a29e59aa9c6da824b6248");
    private static final byte[] BACKEND_KID = HEX.parseHex("12121212");

    @Test
    void backendSignsAndFirmwareVerifies() throws Exception {
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

        String base64Payload = Base64.getEncoder().encodeToString(signedCbor);
        byte[] receivedCbor = Base64.getDecoder().decode(base64Payload);

        assertThat(receivedCbor).isEqualTo(signedCbor);
        assertThat(receivedCbor[0] & 0xFF).isEqualTo(0x8A); // array(10)
        Sign1Message sign1 = decodeSign1FromOuterPayload(receivedCbor, coseSignature.length);
        assertThat(sign1.validate(buildVerificationKey(BACKEND_PUBLIC_KEY))).isTrue();
    }

    @Test
    void tamperedPayloadIsRejected() throws Exception {
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

        byte[] tamperedCbor = originalSignedCbor.clone();
        tamperedCbor[18] ^= 0x01;

        Sign1Message sign1 = decodeSign1FromOuterPayload(tamperedCbor, coseSignature.length);
        sign1.SetContent(extractUnsignedPayload(tamperedCbor, coseSignature.length));
        assertThat(sign1.validate(buildVerificationKey(BACKEND_PUBLIC_KEY))).isFalse();
    }

    @Test
    void wrongKeyIsRejected() throws Exception {
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

        Sign1Message sign1 = decodeSign1FromOuterPayload(signedCbor, coseSignature.length);
        assertThat(sign1.validate(buildVerificationKey(HEX.parseHex(
                "98390187359cad019ba905660ff2ac5df1d21cd313ed75ada78b9f42dbbe9e5e")))).isFalse();
    }

    @Test
    void cborWireFormatMatchesFirmwareExpectations() throws Exception {
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

        assertThat(unsignedCbor[0] & 0xFF).isEqualTo(0x89);
        assertThat(signedCbor[0] & 0xFF).isEqualTo(0x8A);
        assertThat((signedCbor[unsignedCbor.length] & 0xE0)).isEqualTo(0x40);
    }

    @Test
    void printExactDownlinkHex() throws Exception {
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

        assertThat(Base64.getEncoder().encodeToString(signedCbor)).isNotBlank();
    }

    private static Sign1Message decodeSign1FromOuterPayload(byte[] signedCbor, int coseLength) throws Exception {
        int bstrHdrLen = coseLength >= 24 ? 2 : 1;
        int coseSigOffset = signedCbor.length - coseLength - bstrHdrLen;
        byte[] coseBytes = new byte[coseLength];
        System.arraycopy(signedCbor, coseSigOffset + bstrHdrLen, coseBytes, 0, coseLength);
        return (Sign1Message) Message.DecodeFromBytes(coseBytes, MessageTag.Sign1);
    }

    private static byte[] extractUnsignedPayload(byte[] signedCbor, int coseLength) {
        int bstrHdrLen = coseLength >= 24 ? 2 : 1;
        byte[] reUnsignedCbor = new byte[signedCbor.length - coseLength - bstrHdrLen];
        reUnsignedCbor[0] = (byte) 0x89;
        System.arraycopy(signedCbor, 1, reUnsignedCbor, 1, reUnsignedCbor.length - 1);
        return reUnsignedCbor;
    }

    private static OneKey buildVerificationKey(byte[] publicKey) throws Exception {
        CBORObject keyMap = CBORObject.NewMap();
        keyMap.Add(KeyKeys.KeyType.AsCBOR(), KeyKeys.KeyType_OKP);
        keyMap.Add(KeyKeys.OKP_Curve.AsCBOR(), KeyKeys.OKP_Ed25519);
        keyMap.Add(KeyKeys.OKP_X.AsCBOR(), CBORObject.FromObject(publicKey));
        return new OneKey(keyMap);
    }
}
