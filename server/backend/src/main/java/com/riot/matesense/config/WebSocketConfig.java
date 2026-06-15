package com.riot.matesense.config;

import com.riot.matesense.entity.UserEntity;
import com.riot.matesense.repository.UserRepository;
import com.riot.matesense.security.CookieJwtExtractor;
import com.riot.matesense.security.JwtService;
import com.riot.matesense.service.AuthService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
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
import org.springframework.security.core.authority.AuthorityUtils;
import org.springframework.web.socket.config.annotation.EnableWebSocketMessageBroker;
import org.springframework.web.socket.config.annotation.StompEndpointRegistry;
import org.springframework.web.socket.config.annotation.WebSocketMessageBrokerConfigurer;

@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {

    @Value("${app.cors.allowed-origins:http://localhost:3000}")
    private String[] allowedOrigins;

    @Autowired
    @Lazy
    private JwtService jwtService;

    @Autowired
    private UserRepository userRepository;

    @Autowired
    private AuthService authService;

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

                    String token = CookieJwtExtractor.extractJwtFromCookie(cookieHeader);

                    if (token != null && jwtService.isTokenValid(token) && authService.isTokenInStore(token)) {
                        String userIdStr = jwtService.extractUsername(token);
                        UserEntity user = userRepository.findById(Long.parseLong(userIdStr)).orElse(null);
                        if (user == null) {
                            throw new org.springframework.security.core.AuthenticationException("User not found") {};
                        }
                        UsernamePasswordAuthenticationToken authToken =
                                new UsernamePasswordAuthenticationToken(user.getEmail(), null,
                                        AuthorityUtils.createAuthorityList("ROLE_" + user.getRole()));
                        accessor.setUser(authToken);
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
        registry.addEndpoint("/ws").setAllowedOrigins(allowedOrigins);
    }
}
