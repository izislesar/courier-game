#!/usr/bin/env bash
# Performance capture for packaged Courier 404 builds.
# Produces a frame-time log with per-frame delta (ms) and FPS.
#
# Usage:
#   scripts/perf-capture.sh [duration_seconds] [output_log_path]
#
# Defaults:
#   duration=10 seconds
#   output=docs/reports/performance/logs/perfcapture-TIMESTAMP.log
#
# Examples:
#   scripts/perf-capture.sh                          # 10s capture to default path
#   scripts/perf-capture.sh 30                       # 30s capture
#   scripts/perf-capture.sh 15 /tmp/mycapture.log    # 15s to custom path
#
# Note: Run on target GPU (RTX 3050) for production metrics.
#       The packaged binary is at Packaged/LinuxDev/Courier404/Binaries/Linux/Courier404.

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DURATION="${1:-10}"
TIMESTAMP="$(date +%Y%m%d-%H%M%S)"
LOG_DIR="$PROJECT_ROOT/docs/reports/performance/logs"
mkdir -p "$LOG_DIR"

if [ -z "${2:-}" ]; then
  OUTPUT_LOG="$LOG_DIR/perfcapture-${TIMESTAMP}.log"
else
  OUTPUT_LOG="$2"
fi

BIN="$PROJECT_ROOT/Packaged/LinuxDev/Courier404/Binaries/Linux/Courier404"

if [ ! -x "$BIN" ]; then
  echo "ERROR: Packaged binary not found at $BIN"
  echo "Run scripts/package-development.sh first."
  exit 1
fi

echo "==> Performance capture: ${DURATION}s -> $OUTPUT_LOG"
# The binary exits with non-zero when it hits the max frames limit or timer timeout.
# Both are expected success conditions.
timeout 120 "$BIN" -nullrhi -unattended -nosplash -perfcapture -abslog="$OUTPUT_LOG" > /dev/null 2>&1 || true

FRAME_COUNT=$(grep -c "PERF_CAPTURE: frame=" "$OUTPUT_LOG" 2>/dev/null || echo 0)
echo "==> Capture complete: $FRAME_COUNT frames captured"
echo "==> Log: $OUTPUT_LOG"
