# Courier 404 — Build and Test Commands

Authoritative local environment record. Do not guess paths; this file records the actual installed engine.

## Environment

- Engine root: `/home/izislesar/UnrealEngine-5.8.2` (UE 5.8.2, source-built, editor binary present)
- Environment file: `.ue-env` (`UE_ROOT`, `UE_VERSION`)
- Toolchain: bundled UE clang 20.1.8 at
  `$UE_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v26_clang-20.1.8-rockylinux8/x86_64-unknown-linux-gnu`
  (do not use Arch system clang)
- Bundled .NET: `$UE_ROOT/Engine/Binaries/ThirdParty/DotNet/10.0/linux-x64/dotnet`

## Build (editor target)

```bash
source .ue-env
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" Courier404Editor Linux Development \
  -Project="$PWD/Courier404.uproject" -WaitMutex
```

Verified 2026-08-26: Result Succeeded (UHT codegen + clang compile/link).

## Build (game target)

```bash
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" Courier404 Linux Development \
  -Project="$PWD/Courier404.uproject" -WaitMutex
```

Note: first game-target build compiles engine runtime targets and is expected to be long; use only when packaging/runtime verification requires it.

## Packaging / performance

```bash
scripts/package-development.sh
```

Builds Courier404 Linux Development and archives a cooked package under `Packaged/LinuxDev`. Measurement protocol and report template: `docs/reports/performance/README.md`.

## Rules

- Never run multiple Unreal builds/cooks/shader compilations/editor instances concurrently.
- Editor target is the default fast verification gate for C++ changes.
- Automation tests (domain logic) will run via:

```bash
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Courier404.uproject" \
  -ExecCmds="Automation RunTests Courier404" -unattended -nopause -nosplash -nullrhi -log
```

(test module registration arrives with the domain-test issues)

## Concurrency

All UE invocations must be serialized. A mutex-guarded `-WaitMutex` flag is used on every Build.sh call.
