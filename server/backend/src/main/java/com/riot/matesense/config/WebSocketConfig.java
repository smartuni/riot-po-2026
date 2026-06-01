package com.riot.matesense.config;

import com.riot.matesense.security.JwtService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Lazy;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.simp.config.ChannelRegistration;
import org.springframework.messaging.simp.config.MessageBrokerRegistry;
import org.springframework.messaging.simp.stomp.StompCommand;
import org.springframework.messaging.simp.stomp.StompHeaderAccessor;
import org.springframework.messaging.support.ChannelInterceptor;
import org.springframework.messaging.support.MessageHeaderAccessor;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.socket.config.annotation.EnableWebSocketMessageBroker;
import org.springframework.web.socket.config.annotation.StompEndpointRegistry;
import org.springframework.web.socket.config.annotation.WebSocketMessageBrokerConfigurer;

import java.util.List;

@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {

    @Autowired
    @Lazy
    private JwtService jwtService;

    @Override
    public void configureClientInboundChannel(ChannelRegistration registration) {
        registration.interceptors(new ChannelInterceptor() {
            @Override
            public Message<?> preSend(Message<?> message, MessageChannel channel) {
                StompHeaderAccessor accessor = MessageHeaderAccessor.getAccessor(message, StompHeaderAccessor.class);

                if (accessor != null && StompCommand.CONNECT.equals(accessor.getCommand())) {
                    // Extract JWT from the Cookie header in native session headers
                    String cookieHeader = null;
                    if (accessor.getNativeHeader("Cookie") != null) {
                        cookieHeader = accessor.getFirstNativeHeader("Cookie");
                    }
                    // Also try from native session attributes (handshake headers)
                    if (cookieHeader == null) {
                        java.util.Map<String, Object> sessionAttributes = accessor.getSessionAttributes();
                        if (sessionAttributes != null && sessionAttributes.get("nativeHeaders") instanceof java.util.Map) {
                            @SuppressWarnings("unchecked")
                            java.util.Map<String, java.util.List<String>> nativeHeaders =
                                (java.util.Map<String, java.util.List<String>>) sessionAttributes.get("nativeHeaders");
                            java.util.List<String> cookies = nativeHeaders.get("cookie");
                            if (cookies != null && !cookies.isEmpty()) {
                                cookieHeader = cookies.get(0);
                            }
                        }
                    }

                    String token = null;
                    if (cookieHeader != null) {
                        for (String cookie : cookieHeader.split(";")) {
                            String trimmed = cookie.trim();
                            if (trimmed.startsWith("jwt=")) {
                                token = trimmed.substring(4);
                                break;
                            }
                        }
                    }

                    if (token != null && jwtService.isTokenValid(token)) {
                        String userEmail = jwtService.extractUsername(token);
                        UsernamePasswordAuthenticationToken authToken =
                                new UsernamePasswordAuthenticationToken(userEmail, null, List.of());
                        accessor.setUser(authToken);
                        SecurityContextHolder.getContext().setAuthentication(authToken);
                    } else {
                        throw new org.springframework.security.core.AuthenticationException("Invalid or missing JWT") {};
                    }
                }

                return message;
            }
        });
    }

    @Override
    public void configureMessageBroker(MessageBrokerRegistry config) {
        config.enableSimpleBroker("/topic"); // or /queue for private
        config.setApplicationDestinationPrefixes("/app");
    }

    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOriginPatterns("*");
    }
}
