# Courier 404 — Testing and Verification

## Completion rule

In orchestrated mode, a worker reporting `done` means candidate implementation only. The orchestrator/integrator owns final acceptance.

An issue is complete only after:
1. implementation;
2. relevant automated checks;
3. build/compile verification where environment permits;
4. targeted gameplay smoke verification for integration work;
5. integration into `main` when work came from a worker;
6. Beads close/update by the orchestrator with a truthful reason.

## Test pyramid for this project

### Domain tests

Use Unreal automation tests for deterministic logic such as:
- contract state transitions;
- payout calculation;
- need thresholds;
- time advancement;
- relationship consequence calculation;
- arrest outcome state changes;
- save serialization assumptions where feasible.

These tests should not require loading the full city map.

### Component/integration tests

Where practical:
- interaction state;
- package pickup/drop;
- save/load round trip;
- vehicle game-facing state;
- contract-world binding.

### Editor/game smoke tests

For integrated issues verify the actual playable path.

## Core gameplay regression matrix

### Normal delivery

- accept;
- pickup;
- cannot complete before pickup;
- correct drop completes;
- reward paid once;
- duplicate drop does not duplicate payout.

### Anonymous contract

- offer appears under configured condition;
- reject leaves game stable;
- accept creates active contract;
- high-risk consequence hook works;
- completion pays configured amount once.

### Needs

- missed short interval does not punish excessively;
- thresholds advance in order;
- eating improves hunger state;
- sleeping improves fatigue;
- starvation can eventually damage/kill after intended prolonged neglect.

### Relationship

- planned interaction is persisted;
- taking conflicting work can create a missed-plan consequence;
- save/load retains result.

### Police

- encounter can start;
- at least one non-arrest outcome works;
- arrest outcome advances/changes state correctly;
- active risky contract responds consistently;
- player returns to stable gameplay state.

### Hostile encounter

- player can avoid/flee in at least one path;
- losing can produce configured consequence;
- lethal outcome reaches death handling;
- no stuck AI after encounter ends.

### Save/load

- save;
- quit/reload or reload level;
- money restored;
- time restored;
- needs restored;
- relationship restored;
- contract state restored or safely reset according to design;
- player resumes in valid location.

## Build commands

The exact command depends on the local UE5 installation.

Agents must discover and record the actual engine path rather than inventing it.

Typical Linux patterns may resemble:

```bash
/path/to/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh Courier404Editor Linux Development /path/to/Courier404.uproject -WaitMutex
```

or project-generation/build tooling appropriate to the installed engine.

Do not paste a guessed engine path into permanent scripts without configuration.

## Verification artifacts

For large integration issues, record in the Beads close reason or issue notes:
- commands run;
- pass/fail;
- important warnings;
- manual smoke path exercised.

## Broken environment rule

If UE5 is unavailable:
- source-level/domain work may continue;
- static analysis/tests that do not require UE may continue;
- the issue cannot be described as fully build-verified;
- create a blocker with exact missing dependency/path/error;
- do not fake a successful build.

## Production severity and release gates

For normal production runs:
- no issue may claim completion while a known P0 regression caused by that issue remains;
- P0 defects affecting launch, input, save integrity, main-loop progression or reproducible crashes are release blockers;
- P1 defects affecting core UX, major presentation, representative performance or main-loop reliability must have an explicit owner/issue before a release candidate;
- subjective feel/visual claims require human evidence.

Release-candidate verification must additionally include:
- Shipping build from a clean state;
- clean-machine/package launch check;
- human full-loop playtest;
- save/load progression check;
- Low + reference-hardware performance evidence;
- package/content/security/license review;
- exact open blocker list.

Write phase-appropriate reports under `docs/reports/`; reports are evidence, not the task source of truth.


## Performance verification

At integration/performance gates, run a representative packaged-build traversal and capture:

- preset;
- resolution;
- `stat unit`;
- `stat gpu`;
- `stat streaming`;
- memory summary;
- largest cooked asset groups;
- packaged build size;
- recurring hitch observations.

Use Unreal Insights/ProfileGPU/Asset Audit when a budget is missed.

Do not close a performance gate based on editor viewport FPS. If automated capture is not possible without disruptive visible-window loops, record the limitation and use one explicit human profiling/playtest gate instead of repeatedly relaunching the game.

## Scalability verification

Required smoke matrix:

1. Low: Lumen off, no required Nanite/VSM dependency, core atmosphere/readability intact.
2. Medium: stable primary quality/performance path.
3. High: optional UE5 features enabled only if they stay inside the high-reference budget.
4. City population scales down on Low without removing critical NPCs or breaking encounters.

Smoke-test important Nanite content with `r.Nanite 0`.

The exact budgets are in `docs/architecture/performance-budget.md`.


## Multi-agent verification ownership

During an orchestrated run:
- workers do not invoke UE project builds, Unreal automation, cook/package, or real-render profiling;
- workers run only narrow non-UE checks and commit candidate implementation;
- the orchestrator merges worker branches into `main` and performs the required UE verification serially;
- Beads closure happens only after integrated-main evidence exists;
- subjective feel/visual acceptance remains a human gate.

This prevents parallel workers from racing shared UE build artifacts or producing contradictory acceptance evidence.
