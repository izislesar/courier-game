# Courier 404 — Agent Operating Manual

## Mission

Courier 404 is in **production-readiness development**. The proven pre-production vertical slice is historical evidence, not the active roadmap.

Current work state is owned by **Beads**. Product gates are defined by `docs/product/production-readiness.md`. Multi-agent execution is coordinated through `scripts/agent-bus.sh` and the role contract in `docs/agent/multi-agent-orchestration.md`.

## Recover before acting

After a new session, `/compact`, provider/model switch, interruption, or stale-summary suspicion, run:

```bash
bd prime
bd ready --json
git status --short
git branch --show-current
git log --oneline -12
```

Then read:

1. `AGENTS.md`;
2. `docs/product/production-readiness.md`;
3. `docs/agent/multi-agent-orchestration.md` if this is an orchestrated session;
4. the assigned Beads issue or worker task;
5. only the product/architecture docs relevant to that scope.

If chat/compaction state disagrees with Beads/git, **Beads/git wins**.

## Role is explicit

There are three execution roles. Never silently switch roles.

### Lead / single-agent

Used when no multi-agent orchestration is active. May claim one ready issue, implement it, verify it, and update Beads according to the active session policy.

### Orchestrator / integrator

Runs from `main`. It owns:
- Beads status/dependencies/claims/closure;
- assignment of independent work to workers;
- overlap prevention;
- integration into `main`;
- all serialized UE build/cook/package/real-render verification;
- final acceptance evidence.

It **does not implement feature code directly** while workers are active, except for coordination/integration-only fixes or an explicit human instruction.

### Worker

Runs only in `agent/w1`, `agent/w2`, or `agent/w3` worktrees. A worker:
- executes exactly one assigned task at a time;
- never changes Beads status/dependencies/claims;
- never touches `main` or another worker branch;
- never performs package/real-render verification;
- commits only its assigned implementation on its own branch;
- reports commit + narrow evidence through the agent bus;
- waits for the next assignment after the orchestrator integrates and resynchronizes it.

See `docs/agent/multi-agent-orchestration.md` for the full protocol.

## Context loading policy

Use layered context. Do not read the repository indiscriminately.

### L0 — always

- `AGENTS.md`
- recovery commands above
- current role contract
- assigned issue/task
- `docs/product/production-readiness.md`

### L1 — when relevant

- gameplay: `docs/product/gameplay.md`
- art/presentation: `docs/product/art-direction.md`
- renderer/performance/assets: `docs/architecture/performance-budget.md`
- NPC/traffic/city: `docs/architecture/city-simulation.md`
- system design: `docs/architecture/ue5-architecture.md`
- testing/release gates: `docs/architecture/testing-and-verification.md`

### L2 — evidence/history only

- `docs/reports/`
- `docs/archive/`
- completed pre-production plans/specs

Reports are evidence, not task state. Archived documents never reactivate work.

## Source of truth

Beads is the authoritative task graph and durable execution memory.

- Do not create Markdown TODO lists or a second roadmap.
- Do not silently add scope.
- Discoveries become Beads issues, preferably linked with `discovered-from`.
- Dependency direction:

```bash
bd dep add <dependent-issue> <required-issue>
```

means the first issue cannot complete/start as intended until the second is satisfied.

In orchestrated mode, **only the orchestrator mutates Beads state**. Workers may read issue text if useful but must not claim, reopen, close, reprioritize, or edit dependencies.

## Multi-agent scheduling invariant

Parallelism is allowed only for independent scopes.

The orchestrator must avoid assigning two workers that are likely to modify:
- the same source files;
- the same subsystem ownership boundary;
- the same generated content/map;
- the same config section;
- the same acceptance evidence artifact.

Prefer 2–3 genuinely independent leaves over maximum worker occupancy.

When fewer than three safe leaves remain, the orchestrator may perform a bounded backlog-decomposition pass on the next broad production issue. It must create only atomic, objectively verifiable work and must not create filler issues simply to keep workers busy.

## Git/worktree integration invariant

Worker branches are long-lived and reused. Therefore integration must preserve worker commit ancestry.

Preferred integration from `main`:

```bash
git diff main...agent/w1 --stat
git diff main...agent/w1
# inspect first
git merge --no-ff --no-edit agent/w1
```

After successful integration and verification, resynchronize that worker only when its worktree is clean and idle:

```bash
git -C ../courier-w1 merge --ff-only main
```

Do **not** routinely cherry-pick worker commits and then continue reusing the stale worker branch. Cherry-picking breaks the simple fast-forward resync invariant.

Never hard-reset a worker worktree while it is `assigned` or `working`.

## UE verification lane

UE build/cook/editor/package/real-render work is serialized through the orchestrator/integrator lane.

Workers must not run:
- `Build.sh` for project targets;
- `UnrealEditor` / `UnrealEditor-Cmd` automation;
- cook/package scripts;
- visible real-render profiling.

Workers may run non-UE narrow checks such as `git diff --check`, script syntax checks, static inspection, deterministic helper tests that do not invoke UE, and other explicitly safe tooling.

The orchestrator runs the required UE verification after integration.

## Human-only verification boundary

Automation may prove structure, state, buildability, deterministic behavior, and measured telemetry. It may not fabricate subjective evidence.

Human verification is required for claims about:
- mouse/camera feel;
- movement/vehicle feel;
- visual lighting correctness;
- presentation/readability;
- perceived hitching/pop-in;
- end-to-end game feel.

Do not repeatedly launch visible Courier404 windows while the user is working. Prefer bounded automation. If subjective evidence is required, create/update the human gate instead of pretending automation proved it.

## Production-readiness objective

Production work prioritizes:
- controls/camera/movement feel;
- vehicle feel and reliability;
- interaction feedback and UX;
- lighting/material/presentation quality;
- living-city density with hierarchical simulation;
- packaged-build performance and hitching;
- persistence/recovery correctness;
- packaging/Shipping/clean-machine reliability;
- content/asset/security discipline;
- repeatable human playtest gates.

Production readiness is **not** uncontrolled feature expansion.

## UE5 architecture rules

- Unreal Engine 5 C++ project.
- Core gameplay/domain logic in C++.
- Blueprints are thin presentation/configuration layers.
- Use Enhanced Input.
- Keep vehicle gameplay behind its facade.
- Use `UGameInstanceSubsystem` / `UWorldSubsystem` only when lifecycle semantics justify them.
- Prefer components for actor-local behavior and services/subsystems for world/application state.
- Use Data Assets/Data Tables where appropriate.
- Use versioned `USaveGame` persistence with explicit migration behavior.
- Avoid giant Blueprint graphs for authoritative game-state semantics.
- Avoid plugins unless required and reviewed for production/licensing.
- Keep public interfaces narrow.

## Visual rules

Target: realistic ordinary city, late-2000s/early-2010s vocabulary, modern high-quality lighting/material presentation, restrained dirty urban atmosphere.

Do not drift into cyberpunk, synthwave, neon-hacker fantasy, VHS horror, PSX demake, or terminal-cliché aesthetics.

> Mundane objects should look expensive to render, not expensive to own.

Placeholder-flat geometry/materials are unfinished production work, not an acceptance state.

## Performance invariant

Read `docs/architecture/performance-budget.md` before renderer/world/asset/NPC/traffic/package work.

Hard rules:
- Low is a supported art target.
- Baseline rendering remains complete with Lumen disabled.
- Nanite/VSM/Lumen are optional quality layers, not baseline dependencies.
- No experimental renderer feature becomes a shipping dependency without measured evidence.
- Ordinary prop textures default to 512–1024; 2048 normal upper bound for important assets; 4096 requires justification; 8192 shipping content prohibited.
- Avoid unrestricted Tick on ambient actors.
- Packaged builds, not editor FPS, are production performance evidence.

Generated District01 movable lighting remains an explicit exception documented in `docs/decisions/active-exceptions.md`, not a blanket policy.

## Living-city invariant

Read `docs/architecture/city-simulation.md` before population/traffic work.

Use hierarchical simulation:
- off-screen citizens: persistent records;
- visible background population: lightweight agents/Mass where useful;
- nearby/important NPCs: full Character/Pawn behavior.

Do not model every citizen as an always-ticking `ACharacter`.

## Code quality

- Small focused files and narrow ownership.
- Deterministic domain logic where feasible.
- Data-driven definitions where useful.
- No silent catch-all error handling.
- Avoid enormous managers.
- Buildable integrated checkpoints.
- Atomic worker commits.
- No credentials/secrets in tracked files.

## Completion protocol

A worker completion means **candidate implementation**, not issue closure.

Worker completion requires:
1. assigned scope implemented;
2. narrow safe checks run;
3. clean focused diff;
4. local worker commit;
5. result reported through `scripts/agent-bus.sh`.

Issue completion is owned by the orchestrator and requires:
1. worker branch inspected;
2. clean integration into `main`;
3. required UE build/tests/package/render checks serialized in main;
4. human-only verification explicitly recorded if still required;
5. Beads updated/closed with exact evidence;
6. worker acknowledged and resynchronized;
7. `bd ready --json` re-queried.

For long-running orchestration use `docs/agent/night-run.md`.
