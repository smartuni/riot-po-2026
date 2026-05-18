# ==========================================
# 1. Build Stage
# ==========================================
FROM node:24-alpine AS build

WORKDIR /app

# Copy package files and install all dependencies (devDeps needed for build)
COPY package*.json ./
RUN npm ci

# Copy the rest of the application
COPY . .

# Build the Vite application for production
RUN npm run build

# ==========================================
# 2. Production Stage
# ==========================================
FROM nginx:alpine AS production

# Install envsubst for runtime env variable injection
RUN apk add --no-cache gettext

# Copy nginx configuration template (processed by nginx entrypoint)
COPY nginx.conf.template /etc/nginx/templates/default.conf.template

# Copy built assets from build stage
COPY --from=build /app/build /usr/share/nginx/html

EXPOSE 80