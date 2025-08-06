package com.riot.matesense.model;

import lombok.Getter;
import lombok.Setter;

/**
 * A DTO for the UserDetailResponse
 */
@Getter
@Setter
public class UserDetailsResponse {
    private String name;
    private String email;
    private String role;
    private Long workerId;

    public UserDetailsResponse(String name, String email, String role, Long workerId) {
        this.name = name;
        this.email = email;
        this.role = role;
        this.workerId = workerId;
    }
}
