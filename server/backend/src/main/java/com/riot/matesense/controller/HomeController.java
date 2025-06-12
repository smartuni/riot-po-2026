package com.riot.matesense.controller;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class HomeController {
    @GetMapping("/")
    public String home() {
        return "Hello, Unsecured MateSense!";
    }

    @GetMapping("/secured")
    public String secured() {
        return "Hello secured MateSense User!";
    }

}
