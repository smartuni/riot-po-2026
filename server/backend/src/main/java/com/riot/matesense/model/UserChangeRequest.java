package com.riot.matesense.model;

/**
 * A DTO for the UserChangeRequest
 */
public class UserChangeRequest {
    private String name;
    private String password;
    private String newPassword;
    
    public String getName() {
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public String getPassword() {
        return password;
    }
    public void setPassword(String password) {
        this.password = password;
    }
    public String getNewPassword() {
        return newPassword;
    }
    public void setNewPassword(String newPassword) {
        this.newPassword = newPassword;
    }

    
}
