# 2026-08-26 — Pre-Production Gate Verification (RTX 3050)

## Verified (automated)

| Gate | Result |
|---|---|
| Editor target build | Succeeded |
| Automation suite | **45/45 pass** |
| SliceLifecycle commandlet | PASS (exit 0) |
| VerifyDistrict commandlet | PASS (exit 0) |
| GenerateInputAssets commandlet | PASS (exit 0) |
| Packaged LinuxDev build | Present (newer than all source changes) |
| GPU renderer boot (RTX 3050) | Confirmed |
| District01 load in packaged build | Confirmed |
| INPUT_OK / LIGHTS_OK | Confirmed in packaged boot |

## GPU renderer evidence (from boot log)

- **GPU**: NVIDIA GeForce RTX 3050 Laptop GPU (4096 MB VRAM)
- **Driver**: 610.57.04 (NVIDIA UNIX Open Kernel Module)
- **RHI**: Vulkan (VULKAN_SM5), discrete GPU (not integrated)
- **Ray tracing**: Disabled (r.RayTracing=0) — as designed
- **Texture pool**: 2800 MB
- **Shader libraries**: Global (5597 unique), Courier404 (1372 unique)
- **PSO hitches on boot**: 50 (30 graphics, 20 compute), 6 precached
- **Swapchain**: VK_PRESENT_MODE_IMMEDIATE_KHR, 3 images
- **LoadMap time**: ~0.11-0.14s
- **Engine init time**: ~2.07s

## Unresolved (requires interactive playtest)

Frame-time metrics (Game/Render/GPU thread ms, FPS) could not be captured
headlessly — the `stat` overlay renders to the viewport which has no display
in this environment, and the CSV profiler did not flush before process exit.
Per `docs/architecture/testing-and-verification.md`, the exact missing
dependency is recorded instead of fabricating results.

## What requires your interactive visual playtest

Launch the packaged build on the RTX 3050 machine and verify:

1. **Frame performance**: `stat unit` / `stat gpu` / `stat fps` — confirm
   Game Thread ≤ 12ms, Render Thread ≤ 12ms, GPU ≤ 28ms sustained @Low 30fps
2. **Lighting identity**: Day/night cycle looks correct (sun + sky present,
   comps=10, nonmovable=0 at boot) — no blown-out or pitch-black scenes
3. **Input feel**: Enhanced Input responsive — phone (Tab), interact (E),
   vehicle enter/exit, package pickup/delivery all feel immediate
4. **Streaming/hitching**: Drive across District01 — no visible pop-in or
   hitching beyond initial PSO warmup
5. **Visual quality**: Materials, lighting, exposure look like the intended
   mundane-realistic aesthetic (not placeholder-flat)
6. **Full loop playtest**: Accept job → pickup → drive → deliver → get paid →
   buy food → sleep → next day — confirm no softlocks or missing feedback
7. **Save/load**: Save mid-session, reload — confirm money/clock/needs/contracts
   restore correctly, no duplicate rewards

## Evidence files

- `docs/reports/performance/logs/gpu-boot-20260826-173147.log` — full GPU boot
- `docs/reports/performance/logs/gpu-stat-20260826-173332.log` — stat attempt
- `docs/reports/performance/logs/package-linuxdev-20260826-162419.log` — package
- `docs/reports/preprod-status.md` — previous gate report
