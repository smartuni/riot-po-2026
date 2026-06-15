package com.riot.matesense.controller;

import com.riot.matesense.model.AuthRequest;
import com.riot.matesense.model.RegisterRequest;
import com.riot.matesense.model.UserDetailsResponse;
import com.riot.matesense.model.UserChangeRequest;
import com.riot.matesense.security.CookieJwtExtractor;
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
    public UserDetailsResponse login(@RequestBody AuthRequest request, HttpServletResponse response) {
        String token = authService.handleLogin(request);
        ResponseCookie cookie = ResponseCookie.from("jwt", token)
                .httpOnly(true)
                .secure(cookieSecure)
                .sameSite("Lax")
                .path("/")
                .maxAge(36000)
                .build();
        response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());
        return authService.getUserDetails(token);
    }

    @PostMapping("/register")
    public UserDetailsResponse register(@RequestBody RegisterRequest request, HttpServletResponse response) {
        String token = authService.handleRegister(request);
        ResponseCookie cookie = ResponseCookie.from("jwt", token)
                .httpOnly(true)
                .secure(cookieSecure)
                .sameSite("Lax")
                .path("/")
                .maxAge(36000)
                .build();
        response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());
        return authService.getUserDetails(token);
    }

    @PostMapping("/logout")
    public void logout(HttpServletRequest request, HttpServletResponse response) {
        try {
            String token = extractJwt(request);
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
        return authService.getUserDetails(extractJwt(request));
    }

    @PutMapping("/user-change")
    public void changeUserDetails(@RequestBody UserChangeRequest changeRequest, HttpServletRequest request) {
        authService.changeUserDetails(changeRequest, extractJwt(request));
    }

    private String extractJwt(HttpServletRequest request) {
        String token = extractJwtFromCookie(request);
        if (token != null) {
            return token;
        }
        throw new RuntimeException("No authentication token found");
    }

    private String extractJwtFromCookie(HttpServletRequest request) {
        String cookieHeader = request.getHeader("Cookie");
        return CookieJwtExtractor.extractJwtFromCookie(cookieHeader);
    }

}
