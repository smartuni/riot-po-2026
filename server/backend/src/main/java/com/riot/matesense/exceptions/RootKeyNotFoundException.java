package com.riot.matesense.exceptions;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

@ResponseStatus(HttpStatus.NOT_FOUND)
public class RootKeyNotFoundException extends RuntimeException {
    public RootKeyNotFoundException() {
        super("Root key has not been set up yet.");
    }
}
