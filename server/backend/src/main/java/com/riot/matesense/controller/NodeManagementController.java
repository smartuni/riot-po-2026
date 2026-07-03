package com.riot.matesense.controller;

import com.riot.matesense.model.Node;
import com.riot.matesense.model.NodeRequest;
import com.riot.matesense.model.RootKey;
import com.riot.matesense.service.NodeManagementService;
import jakarta.validation.Valid;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("nodes")
public class NodeManagementController {

    private final NodeManagementService nodeManagementService;

    public NodeManagementController(NodeManagementService nodeManagementService) {
        this.nodeManagementService = nodeManagementService;
    }

    @PostMapping("/root-key")
    @PreAuthorize("hasRole('controller')")
    public void uploadRootKey(@RequestBody RootKey rootKey) {
        nodeManagementService.saveRootKey(rootKey);
    }

    @GetMapping("/root-key")
    public RootKey getRootKey() {
        return nodeManagementService.getRootKey();
    }

    @GetMapping
    public List<Node> getAllNodes() {
        return nodeManagementService.getAllNodes();
    }

    @PostMapping
    @PreAuthorize("hasRole('controller')")
    public Node addNode(@Valid @RequestBody NodeRequest body) {
        return nodeManagementService.addNode(body.getName(), body.getPublicKey());
    }

    @DeleteMapping("/{id}")
    @PreAuthorize("hasRole('controller')")
    public void deleteNode(@PathVariable Long id) {
        nodeManagementService.deleteNode(id);
    }
}
