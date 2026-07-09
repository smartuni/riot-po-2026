package com.riot.matesense.exceptions;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

@ResponseStatus(HttpStatus.NOT_FOUND)
public class NodeNotFoundException extends RuntimeException {
    private final Long nodeId;

    public NodeNotFoundException(Long nodeId) {
        super(String.format("Could not find node with id %d.", nodeId));
        this.nodeId = nodeId;
    }

    public Long getNodeId() {
        return nodeId;
    }
}
