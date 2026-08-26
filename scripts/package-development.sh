#!/usr/bin/env bash
# Build + package a Linux Development build of Courier 404.
# Serializes all UE invocations; never run concurrently with other UE builds/cooks.
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$PROJECT_ROOT/.ue-env"

ARCHIVE_DIR="$PROJECT_ROOT/Packaged/LinuxDev"
LOG_DIR="$PROJECT_ROOT/docs/reports/performance/logs"
mkdir -p "$ARCHIVE_DIR" "$LOG_DIR"

echo "==> Building Courier404 Linux Development (game target)"
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" Courier404 Linux Development \
  -Project="$PROJECT_ROOT/Courier404.uproject" -WaitMutex \
  2>&1 | tee "$LOG_DIR/build-game-dev-$(date +%Y%m%d-%H%M%S).log"

echo "==> Cooking + staging Linux Development package"
"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
  -project="$PROJECT_ROOT/Courier404.uproject" \
  -noP4 -platform=Linux -clientconfig=Development \
  -build -cook -pak -compressed -stage \
  -archive -archivedirectory="$ARCHIVE_DIR" \
  2>&1 | tee "$LOG_DIR/package-linuxdev-$(date +%Y%m%d-%H%M%S).log"

echo "==> Package archived under $ARCHIVE_DIR"
du -sh "$ARCHIVE_DIR"
