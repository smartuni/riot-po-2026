package com.riot.matesense.service;

import com.riot.matesense.config.JWTSecretProperties;
import com.riot.matesense.config.TestAccountProperties;
import com.riot.matesense.entity.UserEntity;
import com.riot.matesense.model.AuthRequest;
import com.riot.matesense.model.RegisterRequest;
import com.riot.matesense.model.UserChangeRequest;
import com.riot.matesense.model.AuthResponse;
import com.riot.matesense.model.UserDetailsResponse;
import com.riot.matesense.repository.UserRepository;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.Jwts;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;


import java.security.Key;
import java.util.Date;
import java.util.HashMap;

@Service
public class AuthService {

    private UserRepository userRepository;
    private HashMap<String, Long> tokenStore = new HashMap<>();
    private final Key secretKey; // shared secret with JwtService
    private final BCryptPasswordEncoder passwordEncoder;

    @Autowired
    public AuthService(TestAccountProperties testAccountProperties, UserRepository userRepository,
                       JWTSecretProperties jwtSecretProperties, BCryptPasswordEncoder passwordEncoder) {
        this.userRepository = userRepository;
        this.secretKey = jwtSecretProperties.getKey();
        this.passwordEncoder = passwordEncoder;
        for (TestAccountProperties.Account a : testAccountProperties.getAccounts()) {
            try {
                System.out.println("WARNING! enabling test user account " + a.getEmail() + " -> disable this before deployment!");
                // Check if user already exists (from Flyway migration)
                if (userRepository.findByEmail(a.getEmail()) == null) {
                    RegisterRequest rr = new RegisterRequest();
                    rr.setEmail(a.getEmail());
                    rr.setName(a.getUsername());
                    rr.setPassword(a.getPassword());
                    rr.setRole(a.getRole());
                    this.handleRegisterWithoutDuplicate(rr);
                } else {
                    System.out.println("Test account " + a.getEmail() + " already exists, skipping creation");
                }
            } catch (RuntimeException e) {
                System.out.println("Could not create test account " + a.getEmail() + ": " + e.getMessage());
            }
        }
    }

    public AuthResponse handleLogin(AuthRequest request) {
        UserEntity user = userRepository.findByEmail(request.getEmail());

        if (user == null || !passwordEncoder.matches(request.getPassword(), user.getPassword())) {
            throw new RuntimeException("Invalid credentials");
        }

        String token = generateToken(user.getId());
        tokenStore.put(token, user.getId());
        // System.out.println("Created Login Token " + token);
        return new AuthResponse(token);
    }

    public AuthResponse handleRegister(RegisterRequest request) {
        if (userRepository.findByEmail(request.getEmail()) != null) {
            throw new RuntimeException("Email already in use");
        }
        String hashedPassword = passwordEncoder.encode(request.getPassword());

        UserEntity user = new UserEntity(request.getEmail(), hashedPassword, request.getName(), request.getRole());
        userRepository.save(user);

        String token = generateToken(user.getId());
        tokenStore.put(token, user.getId());
        // System.out.println("Created Register Token " + token);
        return new AuthResponse(token);
    }

    // Helper method for test account initialization - silently skips if email exists
    private AuthResponse handleRegisterWithoutDuplicate(RegisterRequest request) {
        UserEntity existingUser = userRepository.findByEmail(request.getEmail());
        if (existingUser != null) {
            // User already exists, skip creation
            String token = generateToken(existingUser.getId());
            tokenStore.put(token, existingUser.getId());
            return new AuthResponse(token);
        }
        
        String hashedPassword = passwordEncoder.encode(request.getPassword());
        UserEntity user = new UserEntity(request.getEmail(), hashedPassword, request.getName(), request.getRole());
        userRepository.save(user);

        String token = generateToken(user.getId());
        tokenStore.put(token, user.getId());
        return new AuthResponse(token);
    }

    public void handleLogout(String token) {
        if (token == null || !token.contains(" ")) {
            throw new RuntimeException("Invalid token format");
        }
        try {
            String splitTok = token.split(" ")[1];
            tokenStore.remove(splitTok);
            // System.out.println(tokenStore);
            // System.out.println("Logged out token: " + token);
        } catch (ArrayIndexOutOfBoundsException e) {
            throw new RuntimeException("Invalid token format");
        }
    }

    public Long getUserIdFromToken(String token) {
        Claims claims = Jwts.parserBuilder()
                .setSigningKey(secretKey)
                .build()
                .parseClaimsJws(token)
                .getBody();
        return Long.parseLong(claims.getSubject());
    }

    public UserDetailsResponse getUserDetails(String token) {
        if (token == null || !token.contains(" ")) {
            throw new RuntimeException("Invalid token format");
        }
        try {
            String splitTok = token.split(" ")[1];
            Long uId = getUserIdFromToken(splitTok);
            UserEntity user = userRepository.getById(uId);
            if (user == null) {
                throw new RuntimeException("User not found");
            }
            // System.out.println(user.getEmail());
            return new UserDetailsResponse(user.getName(), user.getEmail(), user.getRole(), user.getId());
        } catch (ArrayIndexOutOfBoundsException e) {
            throw new RuntimeException("Invalid token format");
        }
    }

    public void changeUserDetails(UserChangeRequest request, String token) {
        if (token == null || !token.contains(" ")) {
            throw new RuntimeException("Invalid token format");
        }
        try {
            String splitTok = token.split(" ")[1];
            Long uId = getUserIdFromToken(splitTok);
            UserEntity user = userRepository.getById(uId);
            if (user == null || !passwordEncoder.matches(request.getPassword(), user.getPassword())) {
                throw new RuntimeException("Invalid credentials");
            }
            if (!user.getName().equals(request.getName())) {
                user.setName(request.getName());
            }
            if (request.getNewPassword() != null && !request.getNewPassword().isBlank()) {
                if (!passwordEncoder.matches(request.getNewPassword(), user.getPassword())) {
                    String hashedNewPassword = passwordEncoder.encode(request.getNewPassword());
                    user.setPassword(hashedNewPassword);
                }
            }

            userRepository.save(user);
        } catch (ArrayIndexOutOfBoundsException e) {
            throw new RuntimeException("Invalid token format");
        }
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
