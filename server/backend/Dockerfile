# Multi-stage build for Spring Boot Backend
# Stage 1: Build Stage
FROM maven:3.9-eclipse-temurin-17 AS builder

WORKDIR /app

# Copy Maven configuration files
COPY pom.xml .
COPY mvnw .
COPY mvnw.cmd .

# Copy source code
COPY src ./src

# Build the application
RUN mvn clean package -DskipTests

# Stage 2: Runtime Stage
FROM eclipse-temurin:17-jre

WORKDIR /app

# Copy the built JAR from builder stage
COPY --from=builder /app/target/*.jar app.jar

# Install curl for health checks
RUN apt-get update && apt-get install -y --no-install-recommends curl && \
    rm -rf /var/lib/apt/lists/*

# Create non-root user for security
RUN groupadd -g 1001 appuser && \
    useradd -u 1001 -g appuser appuser && \
    chown -R appuser:appuser /app

USER appuser

# Expose port 8080
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/actuator/health || exit 1

# Start the Spring Boot application
ENTRYPOINT ["java", "-jar", "app.jar"]
