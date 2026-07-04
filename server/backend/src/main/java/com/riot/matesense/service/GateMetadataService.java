package com.riot.matesense.service;

import com.riot.matesense.entity.GateMetadataEntity;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.repository.GateMetadataRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class GateMetadataService {

    @Autowired
    GateMetadataRepository gateMetadataRepository;

    @Autowired
    GateService gateService;

    @Autowired
    private SimpMessagingTemplate messagingTemplate;

    /**
     * a method to get all metadata entries for a given gate
     * @param gateId of the gate
     * @return a list with all the metadata entries for the gate
     */
    public List<GateMetadataEntity> getMetadataByGateId(Long gateId) {
        return gateMetadataRepository.findByGateId(gateId);
    }

    /**
     * a method to add a metadata entry for a gate
     * @param gateId of the gate
     * @param key of the metadata entry
     * @param value of the metadata entry
     * @return the saved metadata entry
     * @throws GateNotFoundException if the gate does not exist
     */
    public GateMetadataEntity addMetadata(Long gateId, String key, String value) throws GateNotFoundException {
        gateService.getGateEntityById(gateId);
        GateMetadataEntity metadata = new GateMetadataEntity(gateId, key, value);
        GateMetadataEntity saved = gateMetadataRepository.save(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + gateId, saved);
        return saved;
    }

    /**
     * a method to update a metadata entry
     * @param metadataId of the metadata entry that should be updated
     * @param key the new key
     * @param value the new value
     * @return the updated metadata entry
     * @throws GateNotFoundException if the metadata entry does not exist
     */
    public GateMetadataEntity updateMetadata(Long metadataId, String key, String value) throws GateNotFoundException {
        GateMetadataEntity metadata = gateMetadataRepository.findById(metadataId)
                .orElseThrow(() -> new GateNotFoundException(metadataId));
        metadata.setKey(key);
        metadata.setValue(value);
        GateMetadataEntity saved = gateMetadataRepository.save(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + saved.getGateId(), saved);
        return saved;
    }

    /**
     * a method to remove a metadata entry
     * @param metadataId of the metadata entry that should be removed
     * @throws GateNotFoundException if the metadata entry does not exist
     */
    public void removeMetadata(Long metadataId) throws GateNotFoundException {
        GateMetadataEntity metadata = gateMetadataRepository.findById(metadataId)
                .orElseThrow(() -> new GateNotFoundException(metadataId));
        gateMetadataRepository.delete(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + metadata.getGateId() + "/delete", metadataId);
    }
}
