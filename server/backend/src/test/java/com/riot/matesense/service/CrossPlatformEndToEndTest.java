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

import net.i2p.crypto.eddsa.EdDSASecurityProvider;
import java.security.Security;
import java.util.HexFormat;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Cross-platform end-to-end test that:
 * 1. Simulates request-change -> downlink flow from the backend
 * 2. Signs the gate command with Ed25519 COSE_Sign1
 * 3. Simulates the EXACT C firmware verification logic:
 *    a. Parse CBOR array(10) out of the received bytes
 *    b. Extract unsigned payload (elements 0-8) as array(9)
 *    c. Extract COSE_Sign1 blob (element 9)
 *    d. Decode COSE_Sign1, set external content, verify
 * 4. Outputs exact hex bytes for embedding in C firmware tests
 *
 * This proves the Java backend signing matches the C firmware verification.
 */
class CrossPlatformEndToEndTest {

    private static final HexFormat HEX = HexFormat.of();

    // Backend signing key (matching what is in the DB for the backend service)
    private static final byte[] BACKEND_SEED = HEX.parseHex(
            "ce65cd03fc31cc7137f193e4e0696cf31a15a3507959f5eebdaa849a8cbb7c9d");
    private static final byte[] BACKEND_PUBLIC_KEY = HEX.parseHex(
            "24ccc1fa01cb1e92d541cbac95e6f9e52c16a874b01a29e59aa9c6da824b6248");
    private static final byte[] BACKEND_KID = HEX.parseHex("12121212");

    @Test
    void dumpActualSigStructure() throws Exception {
        byte[] unsignedCbor = FirmwareCborSerializer.serialize(new GateCommandRecord(
                0x01, 0x01, 0x03, BACKEND_KID,
                1L, 1L, 0L,
                new byte[]{0x00, 0x00, 0x00, 0x01}, 0), null);

        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);

        // Decode the COSE_Sign1 to inspect the protected headers
        Sign1Message sign1 = (Sign1Message) Message.DecodeFromBytes(
                coseSignature, MessageTag.Sign1);
        sign1.SetContent(unsignedCbor);
        sign1.setExternal(new byte[0]);

        // Manually construct what we think the Sig_structure is
        String context = "Signature1";
        // Get serialized protected headers by encoding the map
        byte[] protectedHdrs = sign1.getProtectedAttributes().EncodeToBytes();
        byte[] externalData = sign1.getExternal();
        byte[] content = sign1.GetContent();

        System.out.println("--- Sig_structure construction debugging ---");
        System.out.println("Context: " + context);
        System.out.println("Protected bytes (" + protectedHdrs.length + "): " + HEX.formatHex(protectedHdrs));
        System.out.println("External bytes (" + externalData.length + "): " + HEX.formatHex(externalData));
        System.out.println("Content bytes (" + content.length + "): " + HEX.formatHex(content));

        // Build the ToBeSigned array and print it
        com.upokecenter.cbor.CBORObject arr = com.upokecenter.cbor.CBORObject.NewArray();
        arr.Add(context);
        arr.Add(protectedHdrs);
        arr.Add(externalData);
        arr.Add(content);
        byte[] toBeSigned = arr.EncodeToBytes();
        System.out.println("ToBeSigned (" + toBeSigned.length + "): " + HEX.formatHex(toBeSigned));
    }

    @Test
    void debugExternalDataBehavior() throws Exception {
        byte[] unsignedCbor = FirmwareCborSerializer.serialize(new GateCommandRecord(
                0x01, 0x01, 0x03, BACKEND_KID,
                1L, 1L, 0L,
                new byte[]{0x00, 0x00, 0x00, 0x01}, 0), null);

        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);

        Sign1Message sign1 = (Sign1Message) Message.DecodeFromBytes(
                coseSignature, MessageTag.Sign1);
        sign1.SetContent(unsignedCbor);

        System.out.println("After decode, externalData = " + java.util.Arrays.toString(sign1.getExternal()));

        // Verify WITHOUT setting external → sign used empty bstr, verify uses null
        sign1.setExternal(new byte[0]);
        System.out.println("After setExternal(new byte[0]), externalData = " + java.util.Arrays.toString(sign1.getExternal()));
        assertThat(sign1.validate(buildVerificationKey(BACKEND_PUBLIC_KEY))).isTrue();
    }

    @Test
    void simulateFullPipeline() throws Exception {
        // ── Step 1: Create a gate command (simulating request-change flow) ──
        // This is what the backend creates when a user requests a gate state change
        int gateNum = 1;           // Gate #1
        int targetState = 0;       // 0 = OPEN

        GateCommandRecord record = new GateCommandRecord(
                0x01,                      // version
                0x01,                      // messageType (SINGLE_REPORT)
                0x03,                      // recordType (GATE_COMMAND)
                BACKEND_KID,               // writerId = backend KID
                1760000000L,               // sequence (epoch seconds)
                1760000000L,               // hlcPhysical (epoch seconds)
                0L,                        // hlcLogical
                new byte[]{0x00, 0x00, 0x00, (byte) gateNum}, // targetGateId
                targetState                // targetState
        );

        // ── Step 2: Backend signs the gate command ──
        // First, serialize WITHOUT signature → the unsigned payload
        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);

        // Build the COSE_Sign1 signature over the unsigned CBOR
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);

        // Serialize WITH signature → the full wire payload
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Print test vector
        System.out.println("═══ Cross-Platform Test Vector ═══");
        System.out.println("Unsigned CBOR hex: " + HEX.formatHex(unsignedCbor));
        System.out.println("Unsigned length:   " + unsignedCbor.length + " bytes");
        System.out.println("COSE_Sign1 hex:    " + HEX.formatHex(coseSignature));
        System.out.println("COSE_Sign1 length: " + coseSignature.length + " bytes");
        System.out.println("Signed CBOR hex:   " + HEX.formatHex(signedCbor));
        System.out.println("Signed length:     " + signedCbor.length + " bytes");
        System.out.println();

        // ── Step 3: Simulate C firmware verification ──
        // The C firmware receives the signed CBOR and does:

        // 3a. Parse the main CBOR array(10) → extract elements
        assertThat(signedCbor[0] & 0xFF).isEqualTo(0x8A); // array(10)
        assertThat(unsignedCbor[0] & 0xFF).isEqualTo(0x89); // array(9)

        // 3b. Extract the unsigned payload by finding where the signature byte string starts
        // The last element in the array(10) is the COSE_Sign1 byte string
        // We need to find its start by looking for the byte string header after element 8
        int coseSigOffset = findCoseSignatureOffset(signedCbor, unsignedCbor.length);
        assertThat(coseSigOffset).isGreaterThan(0);

        // Extract the re-serialized unsigned payload (first unsignedCbor.length bytes)
        byte[] reUnsignedCbor = new byte[unsignedCbor.length];
        System.arraycopy(signedCbor, 0, reUnsignedCbor, 0, unsignedCbor.length);
        reUnsignedCbor[0] = (byte) 0x89; // fix array header to 9 elements

        // 3c. Verify the re-serialized unsigned CBOR matches the original
        assertThat(reUnsignedCbor).isEqualTo(unsignedCbor);

        // 3d. Decode COSE_Sign1 from the signature bytes
        int bstrHdrLen = coseSignature.length >= 24 ? 2 : 1;
        byte[] coseBytes = new byte[coseSignature.length];
        System.arraycopy(signedCbor, coseSigOffset + bstrHdrLen,
                coseBytes, 0, coseSignature.length);
        Sign1Message sign1 = (Sign1Message) Message.DecodeFromBytes(
                coseBytes, MessageTag.Sign1);

        // 3e. The firmware sets the external payload to the re-serialized unsigned CBOR
        // In Java: we tell the COSE library to use the unsignedCbor as the content
        sign1.SetContent(unsignedCbor);

        // 3f. Verify the signature against the backend public key
        OneKey verificationKey = buildVerificationKey(BACKEND_PUBLIC_KEY);
        boolean verified = sign1.validate(verificationKey);

        // Also print the ToBeSigned for cross-C reference
        com.upokecenter.cbor.CBORObject sigArr = com.upokecenter.cbor.CBORObject.NewArray();
        sigArr.Add("Signature1");
        sigArr.Add(sign1.getProtectedAttributes().EncodeToBytes());
        sigArr.Add(sign1.getExternal());
        sigArr.Add(unsignedCbor);
        byte[] toBeSigned = sigArr.EncodeToBytes();
        System.out.println("ToBeSigned hex: " + HEX.formatHex(toBeSigned));

        System.out.println("═══ Verification Result ═══");
        System.out.println("Signature valid: " + verified);
        assertThat(verified).isTrue();
    }

    @Test
    void tamperedContentIsRejected() throws Exception {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03, BACKEND_KID,
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01}, 0);

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);

        // Tamper with the unsigned content before verification
        byte[] tamperedContent = unsignedCbor.clone();
        tamperedContent[5] ^= 0x01;

        OneKey verificationKey = buildVerificationKey(BACKEND_PUBLIC_KEY);
        Sign1Message sign1 = (Sign1Message) Message.DecodeFromBytes(
                coseSignature, MessageTag.Sign1);
        sign1.SetContent(tamperedContent);
        assertThat(sign1.validate(verificationKey)).isFalse();
    }

    @Test
    void wrongKeyIsRejected() throws Exception {
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03, BACKEND_KID,
                1760000000L, 1760000000L, 0,
                new byte[]{0x00, 0x00, 0x00, 0x01}, 0);

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Use a wrong public key
        byte[] wrongPublicKey = HEX.parseHex(
                "98390187359cad019ba905660ff2ac5df1d21cd313ed75ada78b9f42dbbe9e5e");

        OneKey wrongKey = buildVerificationKey(wrongPublicKey);
        Sign1Message sign1 = decodeSign1(signedCbor, coseSignature.length);
        sign1.SetContent(unsignedCbor);
        assertThat(sign1.validate(wrongKey)).isFalse();
    }

    @Test
    void testVectorRoundtrip() throws Exception {
        // Ensure EdDSA provider is registered (normally done by CoseSign1Encoder static init)
        if (Security.getProvider("EdDSA") == null) {
            Security.addProvider(new EdDSASecurityProvider());
        }

        // Exact hex from the simulateFullPipeline output (gateNum=1, 1760000000L timestamps)
        byte[] testUnsignedCbor = HEX.parseHex(
            "89e1e1e344121212121a68e778001a68e77800004400000001e0");
        byte[] testCoseSignature = HEX.parseHex(
            "8449a20444121212120127a0581a89e1e1e344121212121a68e778001a68e77800004400000001e0" +
            "5840e0eaef969cd9972035f1f4d5fc2390e0f20d6f956785b013ad65e2695cf96ab0" +
            "0ca092e810f8f7ec4587abde6df9019e8a4a701afc84dde19850f0660adbe704");

        System.out.println("Test vector payload: " + HEX.formatHex(testUnsignedCbor));
        System.out.println("Test vector COSE:    " + HEX.formatHex(testCoseSignature));
        System.out.println("Payload length: " + testUnsignedCbor.length + " bytes");

        Sign1Message sign1 = (Sign1Message) Message.DecodeFromBytes(
                testCoseSignature, MessageTag.Sign1);
        sign1.SetContent(testUnsignedCbor);
        sign1.setExternal(new byte[0]);

        var sigField = Sign1Message.class.getDeclaredField("rgbSignature");
        sigField.setAccessible(true);
        byte[] signature = (byte[]) sigField.get(sign1);
        System.out.println("Signature: " + HEX.formatHex(signature));

        // Verify the public key matches
        System.out.println("Expected public key: " + HEX.formatHex(BACKEND_PUBLIC_KEY));

        OneKey vk = buildVerificationKey(BACKEND_PUBLIC_KEY);
        boolean result = sign1.validate(vk);
        System.out.println("Test vector COSE-Java validate: " + result);
        if (!result) {
            // Dump debug info
            System.out.println("ToBeSigned: " + HEX.formatHex(
                CBORObject.NewArray()
                    .Add("Signature1")
                    .Add(sign1.getProtectedAttributes().EncodeToBytes())
                    .Add(sign1.getExternal())
                    .Add(sign1.GetContent())
                .EncodeToBytes()));
        }
        assertThat(result).isTrue();
    }

    @Test
    void cborWireFormatMatchesFirmwareTestVectors() throws Exception {
        // Use the exact same values as the C firmware test vector in test_payloads.c
        // to verify wire format compatibility
        GateCommandRecord record = new GateCommandRecord(
                0x01, 0x01, 0x03,
                new byte[]{0x01, 0x02, 0x03, 0x04},  // writerId matching C test
                0x01,                // sequence = 1 (matches C test)
                0xDACB,              // hlcPhysical = 55995
                0x0A,                // hlcLogical = 10
                new byte[]{0x11, 0x12, 0x13, 0x14},  // gateId matching C test
                0x01                 // state = CLOSED (matches C test)
        );

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);
        byte[] coseSignature = CoseSign1Encoder.buildCoseSign1(
                BACKEND_KID, unsignedCbor, BACKEND_SEED);
        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        // Verify the CBOR structure matches C firmware expectations
        assertThat(unsignedCbor[0] & 0xFF).isEqualTo(0x89); // array(9)
        assertThat(signedCbor[0] & 0xFF).isEqualTo(0x8A); // array(10)

        // Verify first few bytes match the expected structure
        // 0x8A = array(10), 0xE1 = simple(1) [version], 0xE1 = simple(1) [messageType]
        assertThat(signedCbor[1] & 0xFF).isEqualTo(0xE1); // version
        assertThat(signedCbor[2] & 0xFF).isEqualTo(0xE1); // messageType
        assertThat(signedCbor[3] & 0xFF).isEqualTo(0xE3); // recordType = GATE_COMMAND (3)
        assertThat(signedCbor[4] & 0xFF).isEqualTo(0x44); // byte string(4) = writerId
    }

    private static int findCoseSignatureOffset(byte[] signedCbor, int unsignedLen) {
        // The unsigned payload is an array(9) followed by a byte string (COSE signature)
        // We need to find where the COSE signature byte string starts
        // The unsigned payload takes up unsignedLen bytes
        // There might be a byte string header before the actual COSE bytes
        return unsignedLen;
    }

    private static Sign1Message decodeSign1(byte[] signedCbor, int coseLength) throws Exception {
        int bstrHdrLen = coseLength >= 24 ? 2 : 1;
        int coseSigOffset = signedCbor.length - coseLength - bstrHdrLen;
        byte[] coseBytes = new byte[coseLength];
        System.arraycopy(signedCbor, coseSigOffset + bstrHdrLen,
                coseBytes, 0, coseLength);
        return (Sign1Message) Message.DecodeFromBytes(coseBytes, MessageTag.Sign1);
    }

    private static OneKey buildVerificationKey(byte[] publicKey) throws Exception {
        CBORObject keyMap = CBORObject.NewMap();
        keyMap.Add(KeyKeys.KeyType.AsCBOR(), KeyKeys.KeyType_OKP);
        keyMap.Add(KeyKeys.OKP_Curve.AsCBOR(), KeyKeys.OKP_Ed25519);
        keyMap.Add(KeyKeys.OKP_X.AsCBOR(), CBORObject.FromObject(publicKey));
        return new OneKey(keyMap);
    }
}
