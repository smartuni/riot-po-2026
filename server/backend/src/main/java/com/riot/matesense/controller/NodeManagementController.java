package com.riot.matesense.controller;

import com.riot.matesense.model.Node;
import com.riot.matesense.model.RootKey;
import com.riot.matesense.service.NodeManagementService;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/nodes")
public class NodeManagementController {

    private final NodeManagementService nodeManagementService;

    public NodeManagementController(NodeManagementService nodeManagementService) {
        this.nodeManagementService = nodeManagementService;
    }

    @PostMapping("/root-key")
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
    public Node addNode(@RequestBody Map<String, String> body) {
        String name = body.get("name");
        String publicKey = body.get("publicKey");
        return nodeManagementService.addNode(name, publicKey);
    }

    @DeleteMapping("/{id}")
    public void deleteNode(@PathVariable Long id) {
        nodeManagementService.deleteNode(id);
    }
}
