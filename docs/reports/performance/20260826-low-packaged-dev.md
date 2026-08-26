# 2026-08-26 — Low preset, packaged Development build (headless environment)

## Verdict

- Cooked size: **PASS** — 1.1 GiB vs <= 3 GiB target.
- Runtime renderer-dependency smoke: **PASS** — packaged binary boots District01
  to play (`LoadMap` complete, GameMode up, exit clean) with
  `r.DynamicGlobalIlluminationMethod=0` applied from project config; Lumen,
  Nanite and VSM are absent from the baseline path by construction
  (`DefaultScalability.ini` + `DefaultEngine.ini`, see
  `docs/reports/performance/logs/packaged-run.log`).
- GPU frame metrics: **BLOCKED BY ENVIRONMENT** — this host has no GPU/display;
  `-nullrhi` cannot produce meaningful `stat unit/gpu` numbers. Per
  `docs/architecture/testing-and-verification.md` the exact missing dependency
  is recorded instead of fabricating results. Re-run
  `docs/reports/performance/README.md` protocol on GTX 1050 Ti-class hardware.

## Environment

- Host: headless Linux container, 12C/16T, 15.36 GB RAM, no GPU/display.
- Engine: UE 5.8.2 source build, bundled clang 20.1.8 toolchain.
- Build: `scripts/package-development.sh` (Courier404 Linux Development + cook + stage).

## Configuration evidence

- `LogConfig: Set CVar [[r.DynamicGlobalIlluminationMethod:0]]` in packaged log.
- VSM disabled project-wide (`r.Shadow.Virtual.Enable=0`); conventional shadow maps.
- Scalability presets Low/Medium/High defined with texture pools 768/1200/1800/2800 MB
  per budget; FrameTimeCap 33.3 ms; TSR AA; SSR+captures reflections only.
- City Tier-1 agent budgets: Low Day = 10 agents (High Day = 40), night thins further —
  Low reduces density safely without touching critical NPCs (see Courier404.City tests).

## Package size

- Total staged package: 1.1 GiB (`du -sh Packaged/LinuxDev`) — target <= 3 GiB: PASS.
- Largest contributors: engine runtime + shader libs; project content is placeholder
  geometry only. No unused Starter Content or marketplace packs included.

## Frame times / hitching / memory

Not measurable on this host (no GPU). Budgets remain:
- Game Thread <= 12 ms, Render Thread <= 12 ms, GPU <= 28 ms sustained @Low 30 FPS.
- Streaming pool Low: 768 MB.

## Follow-ups

- Owner-hardware run of the README protocol to fill frame-time/hitch/memory tables.
- Any missed target becomes a Beads blocker with exact numbers at that time.
