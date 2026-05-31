package com.riot.matesense.exceptions;

/**
 * Thrown when a login attempt fails because the email/password do not match.
 * Mapped to HTTP 401 with a client-friendly message by {@link ApiExceptionHandler}.
 */
public class InvalidCredentialsException extends RuntimeException {
    public InvalidCredentialsException(String message) {
        super(message);
    }
}
