
package com.riot.matesense.controller;

import com.riot.matesense.entity.GateMetadataEntity;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.service.GateMetadataService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
public class GateMetadataController {

    @Autowired
    GateMetadataService gateMetadataService;

    /**
     * AN API Call to get all metadata entries for a gate
     *
     * @param gateId of the gate
     * @return a list with all the metadata entries for the gate
     */
    @GetMapping("/gates/{gateId}/metadata")
    public List<GateMetadataEntity> getMetadataByGateId(@PathVariable Long gateId) {
        return gateMetadataService.getMetadataByGateId(gateId);
    }

    /**
     * AN API Call to add a metadata entry for a gate
     *
     * @param gateId of the gate
     * @param body containing "key" and "value"
     * @return the saved metadata entry
     * @throws GateNotFoundException if the gate does not exist
     */
    @PostMapping("/gates/{gateId}/metadata")
    public GateMetadataEntity addMetadata(@PathVariable Long gateId, @RequestBody Map<String, String> body)
            throws GateNotFoundException {
        String key = body.get("key");
        String value = body.get("value");
        return gateMetadataService.addMetadata(gateId, key, value);
    }

    /**
     * AN API Call to update a metadata entry
     *
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be updated
     * @param body containing "key" and "value"
     * @return the updated metadata entry
     * @throws GateNotFoundException if the metadata entry does not exist
     */
    @PutMapping("/gates/{gateId}/metadata/{metadataId}")
    public GateMetadataEntity updateMetadata(@PathVariable Long gateId,
                                            @PathVariable Long metadataId,
                                            @RequestBody Map<String, String> body)
            throws GateNotFoundException {
        String key = body.get("key");
        String value = body.get("value");
        return gateMetadataService.updateMetadata(metadataId, key, value);
    }

    /**
     * AN API Call to delete a metadata entry
     *
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be deleted
     * @throws GateNotFoundException if the metadata entry does not exist
     */
    @DeleteMapping("/gates/{gateId}/metadata/{metadataId}")
    public void removeMetadata(@PathVariable Long gateId, @PathVariable Long metadataId)
            throws GateNotFoundException {
        gateMetadataService.removeMetadata(metadataId);
    }
}
