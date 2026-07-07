package com.riot.matesense.service;

import com.riot.matesense.entity.GateMetadataEntity;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.exceptions.MetadataNotFoundException;
import com.riot.matesense.repository.GateMetadataRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

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
     * a method to add a metadata entry for a gate (upsert: updates value if key exists)
     * @param gateId of the gate
     * @param key of the metadata entry
     * @param value of the metadata entry
     * @return the saved metadata entry
     * @throws GateNotFoundException if the gate does not exist
     */
    public GateMetadataEntity addMetadata(Long gateId, String key, String value) throws GateNotFoundException {
        gateService.getGateEntityById(gateId);
        Optional<GateMetadataEntity> existing = gateMetadataRepository.findByGateIdAndKey(gateId, key);
        GateMetadataEntity metadata;
        if (existing.isPresent()) {
            metadata = existing.get();
            metadata.setValue(value);
        } else {
            metadata = new GateMetadataEntity(gateId, key, value);
        }
        GateMetadataEntity saved = gateMetadataRepository.save(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + gateId, saved);
        return saved;
    }

    /**
     * a method to update a metadata entry
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be updated
     * @param key the new key
     * @param value the new value
     * @return the updated metadata entry
     * @throws MetadataNotFoundException if the metadata entry does not exist
     */
    public GateMetadataEntity updateMetadata(Long gateId, Long metadataId, String key, String value) throws MetadataNotFoundException {
        GateMetadataEntity metadata = gateMetadataRepository.findByIdAndGateId(metadataId, gateId)
                .orElseThrow(() -> new MetadataNotFoundException(metadataId));
        metadata.setKey(key);
        metadata.setValue(value);
        GateMetadataEntity saved = gateMetadataRepository.save(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + gateId, saved);
        return saved;
    }

    /**
     * a method to remove a metadata entry
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be removed
     * @throws MetadataNotFoundException if the metadata entry does not exist
     */
    public void removeMetadata(Long gateId, Long metadataId) throws MetadataNotFoundException {
        GateMetadataEntity metadata = gateMetadataRepository.findByIdAndGateId(metadataId, gateId)
                .orElseThrow(() -> new MetadataNotFoundException(metadataId));
        gateMetadataRepository.delete(metadata);
        messagingTemplate.convertAndSend("/topic/gate-metadata/" + gateId + "/delete", metadataId);
    }
}
