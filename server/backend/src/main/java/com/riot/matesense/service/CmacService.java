package com.riot.matesense.service;

import org.bouncycastle.crypto.engines.AESEngine;
import org.bouncycastle.crypto.macs.CMac;
import org.bouncycastle.crypto.params.KeyParameter;
import org.springframework.stereotype.Service;

@Service
public class CmacService {

    private static final int CMAC_LENGTH = 16;

    public byte[] computeCmac(byte[] key, byte[] data) {
        if (key == null || key.length != 16) {
            throw new IllegalArgumentException("AppMACKey must be exactly 16 bytes");
        }
        if (data == null) {
            throw new IllegalArgumentException("Data must not be null");
        }

        CMac cmac = new CMac(new AESEngine(), CMAC_LENGTH * 8);
        cmac.init(new KeyParameter(key));
        cmac.update(data, 0, data.length);

        byte[] tag = new byte[CMAC_LENGTH];
        cmac.doFinal(tag, 0);
        return tag;
    }

    public boolean verifyCmac(byte[] key, byte[] data, byte[] expectedTag) {
        if (expectedTag == null || expectedTag.length != CMAC_LENGTH) {
            return false;
        }
        byte[] computed = computeCmac(key, data);
        return constantTimeEquals(computed, expectedTag);
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
}
