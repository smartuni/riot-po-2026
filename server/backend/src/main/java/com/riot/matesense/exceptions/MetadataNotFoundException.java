package com.riot.matesense.exceptions;

import lombok.EqualsAndHashCode;
import lombok.Value;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

@Value
@EqualsAndHashCode(callSuper=false)
@ResponseStatus(HttpStatus.NOT_FOUND)
public
class MetadataNotFoundException extends Exception {

    private final Long metadataId;

    public MetadataNotFoundException(Long metadataId) {
        super(String.format("Could not find metadata with id %d.", metadataId));
        this.metadataId = metadataId;
    }
}
