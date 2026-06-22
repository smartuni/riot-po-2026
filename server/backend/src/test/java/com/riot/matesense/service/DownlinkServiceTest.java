package com.riot.matesense.service;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertThrows;

class DownlinkServiceTest {

    private static final HexFormat HEX = HexFormat.of();
    private static final byte[] TEST_KEY = HEX.parseHex("2b7e151628aed2a6abf7158809cf4f3c");
    private static final long TEST_HLC = 1760000000L;

    private DownlinkService downlinkService;

    @BeforeEach
    void setUp() {
        CmacService cmacService = new CmacService();
        downlinkService = new DownlinkService(null, null, null, null, cmacService);
    }

    @Test
    void buildSignedDownlink_producesExactly51Bytes() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, new byte[]{0x01});
        assertThat(payload).hasSize(51);
    }

    @Test
    void buildSignedDownlink_containsMessageTypeAtByte0() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, new byte[]{0x01});
        assertThat(payload[0]).isEqualTo((byte) 0x01);
    }

    @Test
    void buildSignedDownlink_containsHlcAtBytes1to4() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, 0x12345678L, 0x0001, new byte[]{0x01});
        assertThat(payload[1] & 0xFF).isEqualTo(0x12);
        assertThat(payload[2] & 0xFF).isEqualTo(0x34);
        assertThat(payload[3] & 0xFF).isEqualTo(0x56);
        assertThat(payload[4] & 0xFF).isEqualTo(0x78);
    }

    @Test
    void buildSignedDownlink_containsCommandIdAtBytes5and6() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0102, new byte[]{0x01});
        assertThat(payload[5] & 0xFF).isEqualTo(0x01);
        assertThat(payload[6] & 0xFF).isEqualTo(0x02);
    }

    @Test
    void buildSignedDownlink_containsCommandDataPaddedTo28Bytes() {
        byte[] cmdData = new byte[]{0x01, 0x02, 0x03};
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, cmdData);
        assertThat(payload[7]).isEqualTo((byte) 0x01);
        assertThat(payload[8]).isEqualTo((byte) 0x02);
        assertThat(payload[9]).isEqualTo((byte) 0x03);
        assertThat(payload[34]).isEqualTo((byte) 0x00);
    }

    @Test
    void buildSignedDownlink_containsCmacAtBytes35to50() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, new byte[]{0x01});
        assertThat(payload).hasSize(51);
        for (int i = 35; i < 51; i++) {
            assertThat(payload[i]).isNotNull();
        }
    }

    @Test
    void buildSignedDownlink_cmacIsValidOverBytes0to34() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, new byte[]{0x01});
        byte[] dataForMac = new byte[35];
        System.arraycopy(payload, 0, dataForMac, 0, 35);
        byte[] tagFromPayload = new byte[16];
        System.arraycopy(payload, 35, tagFromPayload, 0, 16);

        CmacService cmacService = new CmacService();
        byte[] expectedTag = cmacService.computeCmac(TEST_KEY, dataForMac);
        assertThat(HEX.formatHex(tagFromPayload)).isEqualTo(HEX.formatHex(expectedTag));
    }

    @Test
    void buildSignedDownlink_rejectsDataOver28Bytes() {
        byte[] oversized = new byte[29];
        assertThrows(IllegalArgumentException.class,
                () -> downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, oversized));
    }

    @Test
    void buildSignedDownlink_acceptsNullCommandData() {
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, TEST_HLC, 0x0001, null);
        assertThat(payload).hasSize(51);
        assertThat(payload[7]).isEqualTo((byte) 0x00);
    }

    @Test
    void buildSignedDownlink_differentHlcProduceDifferentTags() {
        byte[] p1 = downlinkService.buildSignedDownlink(TEST_KEY, 1000L, 0x0001, new byte[]{0x01});
        byte[] p2 = downlinkService.buildSignedDownlink(TEST_KEY, 2000L, 0x0001, new byte[]{0x01});
        byte[] tag1 = new byte[16];
        byte[] tag2 = new byte[16];
        System.arraycopy(p1, 35, tag1, 0, 16);
        System.arraycopy(p2, 35, tag2, 0, 16);
        assertThat(HEX.formatHex(tag1)).isNotEqualTo(HEX.formatHex(tag2));
    }

    @Test
    void buildSignedDownlink_payloadLengthDoesNotExceed51() {
        byte[] maxData = new byte[28];
        byte[] payload = downlinkService.buildSignedDownlink(TEST_KEY, 0xFFFFFFFFL, 0xFFFF, maxData);
        assertThat(payload.length).isLessThanOrEqualTo(51);
    }
}
