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

# ---- POST-PACKAGE GATES ----
MANIFEST="$ARCHIVE_DIR/Manifest_UFSFiles_Linux.txt"
fail=0

for asset in IMC_CourierDefault IMC_CourierDrive IA_CourierMove IA_PhoneToggle; do
  if ! grep -q "$asset.uasset" "$MANIFEST"; then
    echo "GATE FAIL: $asset missing from cooked manifest"; fail=1
  fi
done

BIN="$ARCHIVE_DIR/Courier404/Binaries/Linux/Courier404"
BOOT_LOG="$LOG_DIR/packaged-boot-$(date +%Y%m%d-%H%M%S).log"
if [ -x "$BIN" ]; then
  timeout 120 "$BIN" -nullrhi -unattended -nosplash -ExecCmds="quit" -abslog="$BOOT_LOG" > /dev/null 2>&1 || true
  grep -q "SLICE_BOOT INPUT_OK" "$BOOT_LOG" || { echo "GATE FAIL: INPUT_BAD in packaged boot"; fail=1; }
  grep -q "LIGHTS_OK" "$BOOT_LOG" || { echo "GATE FAIL: LIGHTS_BAD in packaged boot"; fail=1; }
  grep -q "CITY_OK" "$BOOT_LOG" || { echo "GATE FAIL: CITY_BAD in packaged boot"; fail=1; }
  grep -q "LoadMap: /Game/Maps/District01" "$BOOT_LOG" || { echo "GATE FAIL: District01 not loaded"; fail=1; }
else
  echo "GATE FAIL: packaged binary not found at $BIN"; fail=1
fi

if [ "$fail" -ne 0 ]; then
  echo "==> POST-PACKAGE GATES FAILED"; exit 1
fi
echo "==> POST-PACKAGE GATES PASSED"
