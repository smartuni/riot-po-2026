package com.riot.matesense.service;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;
import static org.junit.jupiter.api.Assertions.assertThrows;

class CmacServiceTest {

    private static final HexFormat HEX = HexFormat.of();

    private static final byte[] RFC4493_KEY = HEX.parseHex("2b7e151628aed2a6abf7158809cf4f3c");

    private CmacService cmacService;

    @BeforeEach
    void setUp() {
        cmacService = new CmacService();
    }

    @Test
    void rfc4493_testVector1_emptyMessage() {
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, new byte[0]);
        assertThat(HEX.formatHex(tag)).isEqualTo("bb1d6929e95937287fa37d129b756746");
    }

    @Test
    void rfc4493_testVector2_16bytes() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        assertThat(HEX.formatHex(tag)).isEqualTo("070a16b46b4d4144f79bdd9dd04a287c");
    }

    @Test
    void rfc4493_testVector3_40bytes() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a" +
                "ae2d8a571e03ac9c9eb76fac45af8e51" +
                "30c81c46a35ce411");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        assertThat(HEX.formatHex(tag)).isEqualTo("dfa66747de9ae63030ca32611497c827");
    }

    @Test
    void rfc4493_testVector4_64bytes() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a" +
                "ae2d8a571e03ac9c9eb76fac45af8e51" +
                "30c81c46a35ce411e5fbc1191a0a52ef" +
                "f69f2445df4f9b17ad2b417be66c3710");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        assertThat(HEX.formatHex(tag)).isEqualTo("51f0bebf7e3b9d92fc49741779363cfe");
    }

    @Test
    void verify_returnsTrue_forValidTag() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        assertThat(cmacService.verifyCmac(RFC4493_KEY, msg, tag)).isTrue();
    }

    @Test
    void verify_returnsFalse_forBitFlippedInMessage() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        msg[0] ^= 0x01;
        assertThat(cmacService.verifyCmac(RFC4493_KEY, msg, tag)).isFalse();
    }

    @Test
    void verify_returnsFalse_forBitFlippedInTag() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        tag[0] ^= 0x01;
        assertThat(cmacService.verifyCmac(RFC4493_KEY, msg, tag)).isFalse();
    }

    @Test
    void verify_returnsFalse_forWrongKey() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        byte[] tag = cmacService.computeCmac(RFC4493_KEY, msg);
        byte[] wrongKey = HEX.parseHex("a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6");
        assertThat(cmacService.verifyCmac(wrongKey, msg, tag)).isFalse();
    }

    @Test
    void verify_returnsFalse_forNullTag() {
        byte[] msg = HEX.parseHex("6bc1bee22e409f96e93d7e117393172a");
        assertThat(cmacService.verifyCmac(RFC4493_KEY, msg, null)).isFalse();
    }

    @Test
    void computeCmac_rejectsNullKey() {
        assertThrows(IllegalArgumentException.class,
                () -> cmacService.computeCmac(null, new byte[0]));
    }

    @Test
    void computeCmac_rejectsShortKey() {
        assertThrows(IllegalArgumentException.class,
                () -> cmacService.computeCmac(new byte[15], new byte[0]));
    }

    @Test
    void computeCmac_rejectsNullData() {
        assertThrows(IllegalArgumentException.class,
                () -> cmacService.computeCmac(RFC4493_KEY, null));
    }
}
