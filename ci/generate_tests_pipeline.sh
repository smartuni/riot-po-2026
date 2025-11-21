#!/usr/bin/env bash
set -euo pipefail

# generate_tests_pipeline.sh
# Generates a GitLab child pipeline YAML with one job per
# subdirectory of TEST_DIR. Expected to be run from the repository root.

TEST_DIR=nodes/firmware/tests
OUT_FILE=${1:-child-pipeline.yml}

if [ ! -d "$TEST_DIR" ]; then
  echo "TEST_DIR '$TEST_DIR' does not exist or is not a directory" >&2
  exit 1
fi

# Start the child pipeline file
cat > "$OUT_FILE" <<'YAML'
stages:
  - test
YAML

# For each test subdir (only those starting with test_), add a new job
found=false
for d in "$TEST_DIR"/test_*; do
  # If the glob didn't match any files, the literal pattern is returned; skip non-existing
  [ -e "$d" ] || continue
  [ -d "$d" ] || continue
  found=true
  name=$(basename "$d" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9_-]/_/g')

  # Extract BOARD variable from Makefile
  board=$(grep -E '^\s*BOARD\s*\?=\s*' "$d/Makefile" | head -n1 | sed -E 's/^\s*BOARD\s*\?=\s*([^[:space:]#]+).*/\1/')

  cat >> "$OUT_FILE" <<YAML
${name}_test:
  stage: test
  image:
    name: \$IMAGE
  rules:
    - when: always
YAML

  # Add tags only if board is not native
  if [ "$board" != "native" ]; then
    cat >> "$OUT_FILE" <<YAML
  tags:
    - hil
YAML
  fi

  cat >> "$OUT_FILE" <<YAML
  script:
    - cd "$d"
    - make clean
    - make all flash
    - make test

YAML
done

if [ "$found" = false ]; then
  echo "No subdirectories found in '$TEST_DIR' - generating an empty pipeline." >&2
fi

echo "Wrote $OUT_FILE"
