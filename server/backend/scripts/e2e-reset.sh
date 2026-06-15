#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"

cd "$repo_root"

# Reset E2E backend to a clean state
# H2 is in-memory so each start gets a fresh database — no volumes to worry about.
# --build rebuilds the backend image so source changes (e.g. the seed) take effect.

docker compose -f server/docker-compose.e2e.yml down

docker compose -f server/docker-compose.e2e.yml up -d --build backend

echo "E2E backend is starting. Use logs to monitor readiness:"
echo "docker compose -f server/docker-compose.e2e.yml logs -f backend"
