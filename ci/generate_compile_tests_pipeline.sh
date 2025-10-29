#!/usr/bin/env bash
set -euo pipefail

# generate_compile_tests_pipeline.sh
# Generates a GitLab child pipeline YAML with one job per
# subdirectory of APPS_DIR. Expected to be run from the repository root.

FIRMWARE_DIR=nodes/firmware
APPS_DIR=${FIRMWARE_DIR}/applications
TESTS_DIR=${FIRMWARE_DIR}/tests
MODULES_DIR=${FIRMWARE_DIR}/custom-modules

OUT_FILE=${1:-compile-tests-pipeline.yml}

if [ ! -d "$APPS_DIR" ]; then
  echo "APPS_DIR '$APPS_DIR' does not exist or is not a directory" >&2
  exit 1
fi

# Start the child pipeline file
cat > "$OUT_FILE" <<'YAML'
stages:
  - test
YAML

# For each test subdir, add a new job
found=false
for d in "$APPS_DIR"/*; do
  [ -d "$d" ] || continue
  found=true
  name=$(basename "$d" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9_-]/_/g')
  cat >> "$OUT_FILE" <<YAML
compile_${name}:
  stage: test
  image:
    name: \$IMAGE
  before_script:
    - mkdir ${MODULES_DIR}/key-distro/include/secrets
    - cp ${TESTS_DIR}/keys/*.h ${MODULES_DIR}/key-distro/include/secrets/
    - cp ${TESTS_DIR}/keys/*.mk ${MODULES_DIR}/key-distro/ttn_configs/
  rules:
    - when: always

  script:
    - cd "$d"
    - make all
YAML
done

if [ "$found" = false ]; then
  echo "No subdirectories found in '$APPS_DIR' - generating an empty pipeline." >&2
fi

echo "Wrote $OUT_FILE"
