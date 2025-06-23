package com.riot.matesense.model;

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

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public String getRole() {
        return role;
    }

    public void setRole(String role) {
        this.role = role;
    }

    public Long getWorkerId() {
        return workerId;
    }

    public void setWorkerId(Long workerId) {
        this.workerId = workerId;
    }
}
