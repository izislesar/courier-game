# Performance Reports

Reports required by `docs/architecture/performance-budget.md`. One file per measurement run:
`YYYYMMDD-<preset>-<buildtype>.md` using the template below. Raw logs in `logs/`.

## Measurement protocol

1. Produce a packaged Development build: `scripts/package-development.sh` (serialized; never concurrent with other UE work).
2. Launch the packaged binary at the target resolution/preset:
   ```bash
   cd Packaged/LinuxDev/Linux/Courier404/Binaries/Linux
   ./Courier404 -windowed -resx=1920 -resy=1080 -ExecCmds="t.MaxFPS 30, stat unit, stat gpu, stat streaming" -log
   ```
3. Drive one representative traversal route through the district (apartment → car → store → drop-off loop) several minutes.
4. Capture: `stat unit`, `stat gpu`, `stat streaming`, `memreport -full`.
5. Record worst recurring hitch (duration + trigger), not just averages.
6. Record packaged size: `du -sh` of archive + largest cooked asset groups (`Size Map`/Asset Audit in editor if budget missed).

## Report template

```markdown
# <date> <preset> <resolution> <build type>

## Environment
- GPU / CPU / RAM:
- Driver:

## Configuration
- Preset / sg levels applied (from log):
- Lumen/Nanite/VSM state: off/off/off expected on Low

## Frame times (sustained, representative route)
- Game Thread: __ ms (budget: <=12 ms Low)
- Render Thread: __ ms (budget: <=12 ms Low)
- GPU: __ ms (budget: <=28 ms Low)
- Worst recurring hitch:

## Memory/streaming
- Texture pool usage vs budget (Low 768 MB):
- memreport path:

## Package size
- Total cooked size vs target (<=3 GiB pre-prod):
- Largest asset groups:

## Verdict
- PASS / measured blocker (exact numbers + suspected dominant cost)
```

## Rules

- Never close a performance gate from editor viewport FPS.
- Low preset smoke must confirm: Lumen off, no Nanite/VSM dependency, atmosphere intact.
