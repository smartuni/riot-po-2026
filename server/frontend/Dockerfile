# ==========================================
# 1. Development Stage
# ==========================================
FROM node:24-alpine AS development

WORKDIR /app

# Copy package files and install dependencies
COPY package*.json ./
RUN npm install

# Copy the rest of the application
COPY . .

EXPOSE 3000

CMD ["npm", "start"]

# ==========================================
# 2. Build Stage
# ==========================================
FROM development AS build

# Build the React application for production
RUN npm run build

# ==========================================
# 3. Production Stage
# ==========================================
FROM nginx:alpine AS production

# Copy the built assets from the build stage to Nginx
COPY --from=build /app/build /usr/share/nginx/html

# Expose port 80 for Nginx
EXPOSE 80

# Start Nginx
CMD ["nginx", "-g", "daemon off;"]
