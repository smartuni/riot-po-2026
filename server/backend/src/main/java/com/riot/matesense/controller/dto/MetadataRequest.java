package com.riot.matesense.controller.dto;

import jakarta.validation.constraints.NotBlank;

public class MetadataRequest {
    @NotBlank(message = "key is required")
    private String key;

    private String value;

    public String getKey() { return key; }
    public void setKey(String key) { this.key = key; }
    public String getValue() { return value; }
    public void setValue(String value) { this.value = value; }
}
