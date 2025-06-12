package com.riot.matesense.service;

import com.riot.matesense.entity.UserEntity;
import com.riot.matesense.model.AuthRequest;
import com.riot.matesense.model.RegisterRequest;
import com.riot.matesense.model.AuthResponse;
import com.riot.matesense.repository.UserRepository;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;

import java.security.Key;
import java.util.Date;
import java.util.HashMap;

@Service
public class AuthService {
    
    @Autowired
    private UserRepository userRepository;

    private HashMap<String, Long> tokenStore = new HashMap<>();

    //same secret key as the JwtService file that validates the key
    private final String secret = "12345678901234567890123456789012";
    private Key secretKey = Keys.hmacShaKeyFor(secret.getBytes());



    public AuthResponse handleLogin(AuthRequest request) {
        UserEntity user = userRepository.findByEmail(request.getEmail());

        if (user == null || !(new BCryptPasswordEncoder().matches(request.getPassword(), user.getPassword()))) {
            throw new RuntimeException("Invalid credentials");
        }

        String token = generateToken(user.getId());
        tokenStore.put(token, user.getId());
        return new AuthResponse(token);
    }

    public AuthResponse handleRegister(RegisterRequest request) {
        if (userRepository.findByEmail(request.getEmail()) != null) {
            throw new RuntimeException("Email already in use");
        }
        String hashedPassword = new BCryptPasswordEncoder().encode(request.getPassword());

        UserEntity user = new UserEntity(request.getEmail(), hashedPassword, request.getName());
        userRepository.save(user);

        String token = generateToken(user.getId());
        tokenStore.put(token, user.getId());
        // System.out.println("CreatedToken" + token);
        return new AuthResponse(token);
    }

    public void handleLogout(String token) {
        tokenStore.remove(token);
        System.out.println("Logged out token: " + token);
    }

    public Long getUserIdFromToken(String token) {
        return tokenStore.get(token); // null if not valid
    }

    public String generateToken(Long userId) {
        long expirationMillis = 1000 * 60 * 60 * 24; // 24 hours
        Date now = new Date();
        Date expiryDate = new Date(now.getTime() + expirationMillis);

        return Jwts.builder()
                .setSubject(String.valueOf(userId))
                .setIssuedAt(now)
                .setExpiration(expiryDate)
                .signWith(secretKey)
                .compact();
    }
}
