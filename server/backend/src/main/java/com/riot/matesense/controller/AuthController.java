package com.riot.matesense.controller;

import com.riot.matesense.model.AuthRequest;
import com.riot.matesense.model.RegisterRequest;
import com.riot.matesense.model.UserDetailsResponse;
import com.riot.matesense.model.UserChangeRequest;
import com.riot.matesense.model.AuthResponse;
import com.riot.matesense.service.AuthService;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpHeaders;
import org.springframework.http.ResponseCookie;
import org.springframework.web.bind.annotation.*;

@RestController
// @CrossOrigin(origins = "*")
@RequestMapping("/auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @Value("${app.cookie.secure:false}")
    private boolean cookieSecure;

    @PostMapping("/login")
    public AuthResponse login(@RequestBody AuthRequest request, HttpServletResponse response) {
        AuthResponse authResponse = authService.handleLogin(request);
        ResponseCookie cookie = ResponseCookie.from("jwt", authResponse.getToken())
                .httpOnly(true)
                .secure(cookieSecure)
                .sameSite("Lax")
                .path("/")
                .maxAge(36000)
                .build();
        response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());
        return authResponse;
    }

    @PostMapping("/register")
    public AuthResponse register(@RequestBody RegisterRequest request, HttpServletResponse response) {
        AuthResponse authResponse = authService.handleRegister(request);
        ResponseCookie cookie = ResponseCookie.from("jwt", authResponse.getToken())
                .httpOnly(true)
                .secure(cookieSecure)
                .sameSite("Lax")
                .path("/")
                .maxAge(36000)
                .build();
        response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());
        return authResponse;
    }

    @PostMapping("/logout")
    public void logout(HttpServletRequest request, HttpServletResponse response) {
        try {
            String token = extractBearerToken(request);
            authService.handleLogout(token);
        } catch (RuntimeException e) {
            // ignore — we clear cookie anyway
        }
        ResponseCookie cookie = ResponseCookie.from("jwt", "")
                .httpOnly(true)
                .secure(cookieSecure)
                .sameSite("Lax")
                .path("/")
                .maxAge(0)
                .build();
        response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());
    }

    @GetMapping("/user-details")
    public UserDetailsResponse userDetails(HttpServletRequest request) {
        return authService.getUserDetails(extractBearerToken(request));
    }

    @PutMapping("/user-change")
    public void changeUserDetails(@RequestBody UserChangeRequest changeRequest, HttpServletRequest request) {
        authService.changeUserDetails(changeRequest, extractBearerToken(request));
    }

    private String extractBearerToken(HttpServletRequest request) {
        String token = extractJwtFromCookie(request);
        if (token != null) {
            return "Bearer " + token;
        }
        String authHeader = request.getHeader("Authorization");
        if (authHeader != null && authHeader.startsWith("Bearer ")) {
            return authHeader;
        }
        throw new RuntimeException("No authentication token found");
    }

    private String extractJwtFromCookie(HttpServletRequest request) {
        String cookieHeader = request.getHeader("Cookie");
        if (cookieHeader != null) {
            for (String cookie : cookieHeader.split(";")) {
                String trimmed = cookie.trim();
                if (trimmed.startsWith("jwt=")) {
                    return trimmed.substring(4);
                }
            }
        }
        return null;
    }

}
