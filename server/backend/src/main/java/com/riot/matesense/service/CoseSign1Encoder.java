package com.riot.matesense.service;

import COSE.AlgorithmID;
import COSE.CoseException;
import COSE.HeaderKeys;
import COSE.KeyKeys;
import COSE.OneKey;
import COSE.Sign1Message;
import com.upokecenter.cbor.CBORObject;
import net.i2p.crypto.eddsa.EdDSASecurityProvider;

import java.security.Security;
import java.util.Arrays;

final class CoseSign1Encoder {

    static {
        if (Security.getProvider("EdDSA") == null) {
            Security.addProvider(new EdDSASecurityProvider());
        }
    }

    private CoseSign1Encoder() {}

    static byte[] buildCoseSign1(byte[] kid, byte[] unsignedCbor, byte[] signingKeySeed) {
        try {
            Sign1Message message = new Sign1Message(false, true);
            message.AddProtected(HeaderKeys.Algorithm, AlgorithmID.EDDSA.AsCBOR());
            message.AddProtected(HeaderKeys.KID, kid);
            message.SetContent(unsignedCbor);
            message.sign(createSigningKey(signingKeySeed));
            return message.EncodeToBytes();
        } catch (CoseException e) {
            throw new IllegalStateException("Failed to build COSE_Sign1 message", e);
        }
    }

    private static OneKey createSigningKey(byte[] signingKeySeed) throws CoseException {
        byte[] seed = normalizeSigningKeySeed(signingKeySeed);
        CBORObject keyMap = CBORObject.NewMap();
        keyMap.Add(KeyKeys.KeyType.AsCBOR(), KeyKeys.KeyType_OKP);
        keyMap.Add(KeyKeys.OKP_Curve.AsCBOR(), KeyKeys.OKP_Ed25519);
        keyMap.Add(KeyKeys.OKP_D.AsCBOR(), CBORObject.FromObject(seed));
        return new OneKey(keyMap);
    }

    private static byte[] normalizeSigningKeySeed(byte[] signingKeySeed) {
        if (signingKeySeed == null) {
            throw new IllegalArgumentException("Ed25519 private key seed must not be null");
        }
        if (signingKeySeed.length == 32) {
            return signingKeySeed.clone();
        }
        if (signingKeySeed.length == 64) {
            return Arrays.copyOf(signingKeySeed, 32);
        }
        throw new IllegalArgumentException("Ed25519 private key seed must be 32 or 64 bytes");
    }
}
