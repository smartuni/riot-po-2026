package com.riot.matesense.exceptions;

import lombok.EqualsAndHashCode;
import lombok.Value;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

@Value
@EqualsAndHashCode(callSuper=false)
@ResponseStatus(HttpStatus.NOT_FOUND)
public
class GateNotFoundException extends Exception {

    private final Long gateId;

    public GateNotFoundException(Long gateId) {
        super(String.format("Could not find gate with id %d.", gateId));
        this.gateId = gateId;
    }
}