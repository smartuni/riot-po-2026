#!/usr/bin/env bash
set -euo pipefail

# generate_web_pipeline.sh
# Generates a GitLab child pipeline YAML for the web services.
# Expected to be run from the repository root.

BACKEND_DIR=server/backend
FRONTEND_DIR=server/frontend
OUT_FILE=${1:-web-pipeline.yml}

if [ ! -d "$BACKEND_DIR" ]; then
  echo "BACKEND_DIR '$BACKEND_DIR' does not exist or is not a directory" >&2
  exit 1
fi

if [ ! -f "$BACKEND_DIR/pom.xml" ]; then
  echo "No pom.xml found in '$BACKEND_DIR'" >&2
  exit 1
fi

if [ ! -f "$BACKEND_DIR/Dockerfile" ]; then
  echo "No Dockerfile found in '$BACKEND_DIR'" >&2
  exit 1
fi

if [ ! -f "$FRONTEND_DIR/Dockerfile" ]; then
  echo "No Dockerfile found in '$FRONTEND_DIR'" >&2
  exit 1
fi

cat > "$OUT_FILE" <<'YAML'
stages:
  - test
  - build

backend_tests:
  stage: test
  image: maven:3.9-eclipse-temurin-17
  variables:
    MAVEN_OPTS: "-Dmaven.repo.local=$CI_PROJECT_DIR/.m2/repository"
  cache:
    key: "$CI_PROJECT_PATH-backend-maven"
    paths:
      - .m2/repository/
  rules:
    - when: always
  script:
    - cd server/backend
    - chmod +x mvnw
    - ./mvnw --batch-mode --errors verify
  artifacts:
    when: always
    reports:
      junit: server/backend/target/surefire-reports/TEST-*.xml
    paths:
      - server/backend/target/surefire-reports/
    expire_in: 1 week

docker_builds:
  stage: build
  image: docker:27-cli
  services:
    - name: docker:27-dind
      command: ["--tls=false"]
  variables:
    DOCKER_HOST: tcp://docker:2375
    DOCKER_TLS_CERTDIR: ""
  rules:
    - when: always
  script:
    - docker version
    - docker build --pull -t riot-po-backend-ci:$CI_COMMIT_SHA server/backend
    - docker build --pull -t riot-po-frontend-ci:$CI_COMMIT_SHA server/frontend
YAML

echo "Wrote $OUT_FILE"
