package com.riot.matesense.service;

import com.riot.matesense.registry.DeviceRegistry;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Base64;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertThrows;

class DownlinkServiceTest {

    private static final HexFormat HEX = HexFormat.of();
    private static final byte[] TEST_KEY = HEX.parseHex("2b7e151628aed2a6abf7158809cf4f3c");
    private static final String TEST_DEVICE = "sensegate-test";

    private DownlinkService downlinkService;
    private DeviceRegistry deviceRegistry;
    private CmacService cmacService;

    @BeforeEach
    void setUp() {
        cmacService = new CmacService();
        deviceRegistry = new DeviceRegistry();
        deviceRegistry.registerDeviceWithKey(TEST_DEVICE, TEST_KEY);
        downlinkService = new DownlinkService(null, null, deviceRegistry, cmacService);
    }

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
        String base64 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        assertThat(base64).isNotNull();
        assertThat(base64).isNotBlank();
        byte[] decoded = Base64.getDecoder().decode(base64);
        assertThat(decoded).isNotEmpty();
    }

    @Test
    void buildSignedGateCommandCbor_fitsIn51Bytes() {
        String base64 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        byte[] decoded = Base64.getDecoder().decode(base64);
        assertThat(decoded.length).isLessThanOrEqualTo(51);
    }

    @Test
    void buildSignedGateCommandCbor_containsSignature() {
        String base64 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        byte[] decoded = Base64.getDecoder().decode(base64);
        assertThat(decoded[0] & 0xFF).isEqualTo(0x8A);

        String base64NoSig = downlinkService.buildSignedGateCommandCbor(1, 1, null);
        byte[] decodedNoSig = Base64.getDecoder().decode(base64NoSig);
        assertThat(decodedNoSig[0] & 0xFF).isEqualTo(0x89);
    }

    @Test
    void buildSignedGateCommandCbor_cmacIsValidOverUnsignedPart() {
        String signedBase64 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        byte[] signedCbor = Base64.getDecoder().decode(signedBase64);

        byte[] unsignedCbor = extractUnsignedCbor(signedCbor);
        byte[] signatureFromPayload = extractSignatureFromSignedCbor(signedCbor);

        byte[] expectedCmac = cmacService.computeCmac(TEST_KEY, unsignedCbor);
        assertThat(HEX.formatHex(signatureFromPayload)).isEqualTo(HEX.formatHex(expectedCmac));
    }

    @Test
    void buildSignedGateCommandCbor_differentStatesProduceDifferentPayloads() {
        String p1 = downlinkService.buildSignedGateCommandCbor(1, 0, TEST_KEY);
        String p2 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        assertThat(p1).isNotEqualTo(p2);
    }

    @Test
    void buildSignedGateCommandCbor_differentGatesProduceDifferentPayloads() {
        String p1 = downlinkService.buildSignedGateCommandCbor(1, 1, TEST_KEY);
        String p2 = downlinkService.buildSignedGateCommandCbor(2, 1, TEST_KEY);
        assertThat(p1).isNotEqualTo(p2);
    }

    private byte[] extractUnsignedCbor(byte[] signedCbor) {
        int sigHeaderSize = 1;
        int sigSize = 16;
        byte[] unsigned = new byte[signedCbor.length - sigHeaderSize - sigSize];
        unsigned[0] = (byte) 0x89;
        System.arraycopy(signedCbor, 1, unsigned, 1, unsigned.length - 1);
        return unsigned;
    }

    private byte[] extractSignatureFromSignedCbor(byte[] signedCbor) {
        byte[] sig = new byte[16];
        System.arraycopy(signedCbor, signedCbor.length - 16, sig, 0, 16);
        return sig;
    }
}
