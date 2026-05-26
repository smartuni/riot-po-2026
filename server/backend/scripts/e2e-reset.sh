#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"

cd "$repo_root"

# Reset E2E DB to a clean, deterministic state
# -v removes the volume so Flyway applies cleanly

docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml down -v

docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml up -d postgres backend

echo "E2E backend is starting. Use logs to monitor readiness:"
echo "docker compose -f server/docker-compose.yml -f server/docker-compose.e2e.yml logs -f backend"
