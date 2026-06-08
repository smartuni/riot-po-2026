package com.riot.matesense.security;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.web.authentication.WebAuthenticationDetailsSource;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import com.riot.matesense.entity.UserEntity;
import com.riot.matesense.repository.UserRepository;
import com.riot.matesense.service.AuthService;

import org.springframework.security.core.authority.AuthorityUtils;

import java.io.IOException;

@Component
public class JwtAuthenticationFilter extends OncePerRequestFilter {

    private final JwtService jwtService;
    private final AuthService authService;
    private final UserRepository userRepository;

    public JwtAuthenticationFilter(JwtService jwtService, AuthService authService, UserRepository userRepository) {
        this.jwtService = jwtService;
        this.authService = authService;
        this.userRepository = userRepository;
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request,
                                    HttpServletResponse response,
                                    FilterChain filterChain)
            throws ServletException, IOException {

        String cookieHeader = request.getHeader("Cookie");
        String token = CookieJwtExtractor.extractJwtFromCookie(cookieHeader);

        if (token != null) {
            String userIdStr = jwtService.extractUsername(token);
            if (userIdStr != null && SecurityContextHolder.getContext().getAuthentication() == null) {
                if (jwtService.isTokenValid(token) && authService.isTokenInStore(token)) {
                    UserEntity user = userRepository.findById(Long.parseLong(userIdStr)).orElse(null);
                    if (user != null) {
                        UsernamePasswordAuthenticationToken authToken =
                                new UsernamePasswordAuthenticationToken(user.getEmail(), null,
                                        AuthorityUtils.createAuthorityList("ROLE_" + user.getRole()));
                        authToken.setDetails(new WebAuthenticationDetailsSource().buildDetails(request));
                        SecurityContextHolder.getContext().setAuthentication(authToken);
                    }
                }
            }
        }
        filterChain.doFilter(request, response);
    }
    @Override
    protected boolean shouldNotFilter(HttpServletRequest request) {
        String path = request.getServletPath();
        return path.equals("/auth/login") || path.equals("/auth/register") || path.equals("/auth/logout");
    }
}
