package com.riot.matesense.security;

public class CookieJwtExtractor {

    public static String extractJwtFromCookie(String cookieHeader) {
        if (cookieHeader == null) {
            return null;
        }
        for (String cookie : cookieHeader.split(";")) {
            String trimmed = cookie.trim();
            if (trimmed.startsWith("jwt=")) {
                return trimmed.substring(4);
            }
        }
        return null;
    }
}
