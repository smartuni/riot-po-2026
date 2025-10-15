#!/usr/bin/env bash
set -euo pipefail

# generate_child_pipeline.sh
# Generates a GitLab child pipeline YAML (child-pipeline.yml) with one job per
# subdirectory of TEST_DIR. Expected to be run from the repository root.

TEST_DIR=${TEST_DIR:-nodes/firmware/tests}
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

# For each test subdir, add a new job
found=false
for d in "$TEST_DIR"/*; do
  [ -d "$d" ] || continue
  found=true
  name=$(basename "$d" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9_-]/_/g')
  cat >> "$OUT_FILE" <<YAML
${name}_test:
  stage: test
  image:
    name: \$IMAGE
  rules:
    - when: always

  script:
    - cd "$d"
    - make clean
    - make all
    - make cleanterm
YAML
done

if [ "$found" = false ]; then
  echo "No subdirectories found in '$TEST_DIR' - generating an empty child pipeline." >&2
fi

echo "Wrote $OUT_FILE"
