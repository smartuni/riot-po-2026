package com.riot.matesense.service;

import com.riot.matesense.config.DownPayload;
import com.riot.matesense.config.MqttProperties;
import com.riot.matesense.mqtt.TTNMqttPublisher;
import com.riot.matesense.registry.DeviceRegistry;
import org.bouncycastle.crypto.params.Ed25519PrivateKeyParameters;
import org.bouncycastle.crypto.signers.Ed25519Signer;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Base64;
import java.util.HexFormat;
import java.util.List;

import static com.riot.matesense.enums.RecordType.GATE_COMMAND;

@Service
public class DownlinkService {

    private final TTNMqttPublisher mqttPublisher;
    private final MqttProperties mqttProperties;
    private final DeviceRegistry deviceRegistry;
    private final KeyStoreService keyStoreService;

    public DownlinkService(TTNMqttPublisher mqttPublisher,
                           MqttProperties mqttProperties, DeviceRegistry deviceRegistry,
                           KeyStoreService keyStoreService) {
        this.mqttPublisher = mqttPublisher;
        this.mqttProperties = mqttProperties;
        this.deviceRegistry = deviceRegistry;
        this.keyStoreService = keyStoreService;
    }

    public void sendDownlinkToDevice(DownPayload payloadData) {
        try {
            List<List<Integer>> sollStatusList = payloadData.getStatuses().stream()
                    .map(statusEntry -> Arrays.asList(statusEntry.get(0), statusEntry.get(1)))
                    .toList();
            for (List<Integer> gateStatePair : sollStatusList) {
                int gateNum = gateStatePair.get(0);
                int targetState = gateStatePair.get(1);
                String base64Payload = buildSignedGateCommandCbor(gateNum, targetState,
                        getServerSigningKeySeed(), getServerKid());

                for (String gateDevice : deviceRegistry.getAllGateDevices()) {
                    String topic = mqttProperties.buildDeviceDownlinkTopic(gateDevice);
                    String ttnJson = buildTtnJsonPayload(base64Payload);
                    mqttPublisher.publishDownlink(ttnJson.getBytes(), topic);
                }
            }
        } catch (Exception e) {
            System.err.println("Fehler beim Downlink-Senden: " + e.getMessage());
        }
    }

    public String sendSignedDownlink(String deviceId, int gateNum, int targetState) {
        String base64Payload = buildSignedGateCommandCbor(gateNum, targetState,
                getServerSigningKeySeed(), getServerKid());

        String ttnJson = buildTtnJsonPayload(base64Payload);

        String topic = mqttProperties.buildDeviceDownlinkTopic(deviceId);
        mqttPublisher.publishDownlink(ttnJson.getBytes(), topic);

        return base64Payload;
    }

    static String buildSignedGateCommandCbor(int gateNum, int targetState,
                                              byte[] signingKeySeed, byte[] kid) {
        GateCommandRecord record = toGateCommandRecord(gateNum, targetState);

        byte[] unsignedCbor = FirmwareCborSerializer.serialize(record, null);

        byte[] rawSig = signEd25519(signingKeySeed, unsignedCbor);
        byte[] coseSignature = CoseSign1Encoder.encode(kid, rawSig);

        byte[] signedCbor = FirmwareCborSerializer.serialize(record, coseSignature);

        return Base64.getEncoder().encodeToString(signedCbor);
    }

    static byte[] signEd25519(byte[] signingKeySeed, byte[] data) {
        Ed25519PrivateKeyParameters privKey = new Ed25519PrivateKeyParameters(signingKeySeed, 0);
        Ed25519Signer signer = new Ed25519Signer();
        signer.init(true, privKey);
        signer.update(data, 0, data.length);
        return signer.generateSignature();
    }

    private byte[] getServerSigningKeySeed() {
        var serverKey = keyStoreService.getServerKey()
                .orElseThrow(() -> new IllegalStateException("No server signing key configured"));
        return Arrays.copyOf(serverKey.getPrivateKey(), 32);
    }

    private byte[] getServerKid() {
        var serverKey = keyStoreService.getServerKey()
                .orElseThrow(() -> new IllegalStateException("No server signing key configured"));
        return HexFormat.of().parseHex(serverKey.getKid());
    }

    static GateCommandRecord toGateCommandRecord(int gateNum, int targetState) {
        long nowMs = System.currentTimeMillis();
        long epochSeconds = nowMs / 1000;

        return new GateCommandRecord(
                0x01,
                0x01,
                GATE_COMMAND.getCode(),
                new byte[]{0x12, 0x12, 0x12, 0x12},
                epochSeconds,
                epochSeconds,
                0,
                new byte[]{0x00, 0x00, 0x00, (byte) gateNum},
                mapGateState(targetState)
        );
    }

    static int mapGateState(int backendState) {
        return switch (backendState) {
            case 0 -> 0;
            case 1 -> 1;
            default -> throw new IllegalArgumentException("Unsupported gate state: " + backendState);
        };
    }

    private String buildTtnJsonPayload(String base64Payload) {
        return String.format("""
                {
                  "downlinks": [
                    {
                      "f_port": 15,
                      "frm_payload": "%s",
                      "priority": "NORMAL"
                    }
                  ]
                }
                """, base64Payload);
    }
}
