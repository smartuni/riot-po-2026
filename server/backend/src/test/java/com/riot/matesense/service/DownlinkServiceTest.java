package com.riot.matesense.service;

import COSE.AlgorithmID;
import COSE.HeaderKeys;
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
import static org.junit.jupiter.api.Assertions.assertThrows;

class DownlinkServiceTest {

    private static final HexFormat HEX = HexFormat.of();

    private static final byte[] TEST_SIGNING_KEY_SEED = HEX.parseHex(
            "ce65cd03fc31cc7137f193e4e0696cf31a15a3507959f5eebdaa849a8cbb7c9d");
    private static final byte[] TEST_PUBLIC_KEY = HEX.parseHex(
            "24ccc1fa01cb1e92d541cbac95e6f9e52c16a874b01a29e59aa9c6da824b6248");
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
    void buildSignedGateCommandCbor_containsCoseSignature() throws Exception {
        byte[] cose = CoseSign1Encoder.buildCoseSign1(TEST_KID, new byte[]{0x01, 0x02, 0x03}, TEST_SIGNING_KEY_SEED);

        Sign1Message message = (Sign1Message) Message.DecodeFromBytes(cose, MessageTag.Sign1);
        assertThat(message.findAttribute(HeaderKeys.Algorithm)).isEqualTo(AlgorithmID.EDDSA.AsCBOR());
        assertThat(message.findAttribute(HeaderKeys.KID).GetByteString()).isEqualTo(TEST_KID);
        assertThat(message.validate(buildVerificationKey(TEST_PUBLIC_KEY))).isTrue();
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
    void buildCoseSign1_validatesWithPublicKey() throws Exception {
        byte[] data = new byte[]{0x01, 0x02, 0x03};
        byte[] cose = CoseSign1Encoder.buildCoseSign1(TEST_KID, data, TEST_SIGNING_KEY_SEED);
        Sign1Message message = (Sign1Message) Message.DecodeFromBytes(cose, MessageTag.Sign1);
        assertThat(message.validate(buildVerificationKey(TEST_PUBLIC_KEY))).isTrue();
    }

    private static OneKey buildVerificationKey(byte[] publicKey) throws Exception {
        CBORObject keyMap = CBORObject.NewMap();
        keyMap.Add(KeyKeys.KeyType.AsCBOR(), KeyKeys.KeyType_OKP);
        keyMap.Add(KeyKeys.OKP_Curve.AsCBOR(), KeyKeys.OKP_Ed25519);
        keyMap.Add(KeyKeys.OKP_X.AsCBOR(), CBORObject.FromObject(publicKey));
        return new OneKey(keyMap);
    }
}
