
package com.riot.matesense.controller;

import com.riot.matesense.controller.dto.MetadataRequest;
import com.riot.matesense.entity.GateMetadataEntity;
import com.riot.matesense.exceptions.GateNotFoundException;
import com.riot.matesense.exceptions.MetadataNotFoundException;
import com.riot.matesense.service.GateMetadataService;
import jakarta.validation.Valid;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

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
    public GateMetadataEntity addMetadata(@PathVariable Long gateId,
                                         @Valid @RequestBody MetadataRequest body)
            throws GateNotFoundException {
        return gateMetadataService.addMetadata(gateId, body.getKey(), body.getValue());
    }

    /**
     * AN API Call to update a metadata entry
     *
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be updated
     * @param body containing "key" and "value"
     * @return the updated metadata entry
     * @throws MetadataNotFoundException if the metadata entry does not exist
     */
    @PutMapping("/gates/{gateId}/metadata/{metadataId}")
    public GateMetadataEntity updateMetadata(@PathVariable Long gateId,
                                            @PathVariable Long metadataId,
                                            @Valid @RequestBody MetadataRequest body)
            throws MetadataNotFoundException {
        return gateMetadataService.updateMetadata(gateId, metadataId, body.getKey(), body.getValue());
    }

    /**
     * AN API Call to delete a metadata entry
     *
     * @param gateId of the gate
     * @param metadataId of the metadata entry that should be deleted
     * @throws MetadataNotFoundException if the metadata entry does not exist
     */
    @DeleteMapping("/gates/{gateId}/metadata/{metadataId}")
    public void removeMetadata(@PathVariable Long gateId, @PathVariable Long metadataId)
            throws MetadataNotFoundException {
        gateMetadataService.removeMetadata(gateId, metadataId);
    }
}
