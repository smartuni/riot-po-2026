package com.riot.matesense.controller;

import com.riot.matesense.model.AuthRequest;
import com.riot.matesense.model.RegisterRequest;
import com.riot.matesense.model.UserDetailsResponse;
import com.riot.matesense.model.UserChangeRequest;
import com.riot.matesense.model.AuthResponse;
import com.riot.matesense.service.AuthService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
// @CrossOrigin(origins = "*")
@RequestMapping("/auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @PostMapping("/login")
    public AuthResponse login(@RequestBody AuthRequest request) {
        return authService.handleLogin(request);
    }

    @PostMapping("/register")
    public AuthResponse register(@RequestBody RegisterRequest request) {
        return authService.handleRegister(request);
    }

    @PostMapping("/logout")
    public void logout(@RequestHeader("Authorization") String token) {
        authService.handleLogout(token);
    }

    @GetMapping("/user-details")
    public UserDetailsResponse userDetails(@RequestHeader("Authorization") String token) {
        return authService.getUserDetails(token);
    }

    @PutMapping("/user-change")
    public void changeUserDetails(@RequestHeader("Authorization") String token, @RequestBody UserChangeRequest request) {
        authService.changeUserDetails(request, token);
    }

}
