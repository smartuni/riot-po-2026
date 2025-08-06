package com.riot.matesense.exceptions;

import lombok.EqualsAndHashCode;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

@EqualsAndHashCode(callSuper=false)
@ResponseStatus(HttpStatus.BAD_REQUEST)
public
class GateAlreadyExistingException extends Exception {

    Long id;
    public GateAlreadyExistingException(Long id) {
        super(String.format("Gate with ID %s does already exist.", id));
        this.id = id;
    }
}