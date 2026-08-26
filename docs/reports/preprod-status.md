# Courier 404 — Pre-Production Vertical Slice Status

**Date:** 2026-08-26 · **Gate:** preprod release gate (jev.20)

## Playable state

A new player launches the packaged/editor build straight into `/Game/Maps/District01`
(apartment PlayerStart) and can play the full loop without editor intervention:

```text
wake in apartment
→ phone (Tab): accept Job.Courier01
→ take parcel at pickup point (E)
→ enter car (E), drive, exit
→ deliver at Point.DropA -> +120 cr exactly once
→ buy meal at store (15 cr), hunger restored
→ sleep in bed -> clock jumps to next-day 07:00, fatigue restored
→ evening: anonymous Job.NightDrop appears (900 cr)
   accept or reject; if accepted:
     locker bag pickup -> risky drop at Point.RiskyDrop
     police patrol zone stop: warning / fine / arrest (flee escalates)
       arrest = fine(clamped) + 8h detention + risky job failed
-> hostile courtyard: warn/pursue/attack; flee works solo, groups injure,
   rob (<45% hp, capped 80cr) and can kill
-> any death routes through unified recovery:
   wake home 08:00 next day, medical cost clamped, active work failed
-> save/load round trip preserves money/clock/needs/relationship/contracts;
   no duplicate rewards across reloads
```

City baseline: 250 persistent citizen records, time-of-day profiles
(Shop/Sit/Smoke day; Sleep night), budgeted lightweight agents
(Low Day = 10 → High Day = 40, ceiling 60), courier-unloading ambient event,
same-zone disturbance reactions. Off-screen citizens are data-only.

## Commands

```bash
source .ue-env
# editor target build (default verification gate)
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" Courier404Editor Linux Development \
  -Project="$PWD/Courier404.uproject" -WaitMutex

# automation suite (43 tests)
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" Courier404.uproject \
  -ExecCmds="Automation RunTests Courier404" -TestExit="Automation Test Queue Empty" \
  -unattended -nopause -nosplash -nullrhi -stdout

# deterministic content generation
... -run=GenerateInputAssets    # input actions/mapping contexts
... -run=BuildDistrict          # rebuild District01 idempotently
... -run=VerifyDistrict         # landmark/rig assertions (exit code)
... -run=SliceLifecycle         # end-to-end lifecycle proof (exit code)

# packaged build + size
scripts/package-development.sh
```

## Verification evidence (this gate run)

| Gate | Result |
|---|---|
| Editor target build | Succeeded |
| Automation suite | 43 pass / 0 fail |
| Slice lifecycle commandlet | SLICE LIFECYCLE PASS (0 failures) |
| District verify commandlet | DISTRICT VERIFY PASS |
| Packaged LinuxDev build | BUILD SUCCESSFUL |
| Cooked size | 1.1 GiB (target ≤ 3 GiB) PASS |
| Renderer-dependency smoke | r.DGIM=0 applied from config; no Lumen/Nanite/VSM in baseline |
| GPU frame metrics | BLOCKED BY ENVIRONMENT (no GPU/display host) |

Details: `docs/reports/performance/20260826-low-packaged-dev.md`.

## Remaining blockers

1. **GPU performance metrics** (jev.23 evidence gap): this host has no GPU/display.
   Frame-time/hitch/streaming tables must be captured on GTX 1050 Ti-class hardware
   using the protocol in `docs/reports/performance/README.md`. Any missed target
   becomes a Beads blocker with exact numbers then.

## Known issues / remaining work (truthful)

- **courier-game-7ew** (backlog): swap lite drivetrain to Chaos Vehicles when authored
  skeletal vehicle assets exist; facade keeps the swap surgical.
- Final-quality meshes/materials/lighting art remain placeholders by design; the
  lighting identity, exposure and mixed urban light families are established and
  clock-driven.
- True-PIE human walkthrough of the checklist is available on any display machine;
  every stage is additionally proven deterministically by `-run=SliceLifecycle`.
- Beads triage at gate time: epic `courier-game-jev` + all 22 slice children closed;
  only `courier-game-7ew` remains intentionally open as external-asset backlog.

## Architecture conformance

- Baseline renderer complete without Lumen/Nanite/VSM (config-enforced).
- Living-city hierarchy: Tier-0 records authoritative; Tier-1 budgeted pool;
  Tier-2 NPCs only where gameplay requires (girlfriend/hostile/police).
- Deterministic domain services (contracts, needs, clock, police, hostile,
  relationship) are plain C++, unit-tested without world/GI; presentation is thin.
