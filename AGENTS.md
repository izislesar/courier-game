# Courier 404 — Agent Operating Manual

## Mission

Courier 404 is now in **production-readiness development**, not pre-production bootstrap.

The completed pre-production vertical slice is historical evidence. Do not resume old slice work merely because an old plan, compacted summary, report, or bootstrap artifact mentions it.

Current task state is determined by **Beads**. Product boundaries and production gates are defined by `docs/product/production-readiness.md`.

## First action in every new, compacted, or model-switched agent context

Run, in this order:

```bash
bd prime
bd ready --json
git status --short
git log --oneline -12
```

Then:

1. read `AGENTS.md`;
2. identify the ready issue you intend to work on;
3. `bd show <issue-id> --json`;
4. read `docs/product/production-readiness.md`;
5. load only the product/architecture docs relevant to that issue;
6. read a linked Superpowers spec/plan if one exists;
7. read the latest verification report only when the issue depends on current playable/build state.

Do **not** trust a conversation summary or compaction summary for current issue state when Beads/git disagree. Recover from the repository.

## Context loading policy

Use layered context instead of reading the entire repository for every issue.

### L0 — always load

- `AGENTS.md`
- `bd prime`
- `bd ready --json`
- chosen Beads issue
- `docs/product/production-readiness.md`

### L1 — load when relevant

- gameplay: `docs/product/gameplay.md`
- art/presentation: `docs/product/art-direction.md`
- renderer/performance/assets: `docs/architecture/performance-budget.md`
- NPC/traffic/city density: `docs/architecture/city-simulation.md`
- code/system design: `docs/architecture/ue5-architecture.md`
- testing/release gates: `docs/architecture/testing-and-verification.md`

### L2 — evidence/history only

- `docs/reports/`
- `docs/archive/`
- completed pre-production plans/specs

Reports and archived documents are evidence, not task trackers.

See `docs/agent/context-loading.md` for the recovery contract.

## Source of truth

Beads is the authoritative task graph and long-horizon execution memory.

- Do not create Markdown TODO lists.
- Do not keep a second task tracker.
- Do not silently add scope.
- All newly discovered work must be represented in Beads.
- New bugs/follow-ups discovered during an issue should use a `discovered-from` relationship where possible.
- Work on ready issues unless resolving a blocker explicitly requires otherwise.
- Do not reopen completed work because an old document or compaction summary mentions it.

Typical workflow:

```bash
bd ready --json
bd show <issue-id> --json
bd update <issue-id> --claim --json
# implement and verify
bd close <issue-id> --reason "Completed and verified: <evidence>" --json
```

Dependency direction:

```bash
bd dep add <dependent-issue> <required-issue>
```

means the first issue cannot start until the second closes.

## Production-readiness objective

The active product objective is to turn the proven vertical slice into a **production-grade game foundation and shippable quality path**.

That means improving and proving, issue by issue:

- player controls, camera and movement feel;
- vehicle feel and reliability;
- interaction feedback and UX;
- visual/material/lighting quality on real renderers;
- living-city density without brute-force simulation;
- performance and hitching on packaged builds;
- save compatibility, migration and corruption handling;
- deterministic gameplay correctness;
- crash/error handling and diagnostics;
- packaging, clean-machine launch and Shipping configuration;
- accessibility/settings/input rebinding where scoped;
- production asset discipline and content validation;
- repeatable human playtest gates.

“Production” does **not** mean uncontrolled feature expansion. Prefer hardening, feel, presentation, reliability, performance and content quality over adding unrelated systems.

Do not claim the game is production-ready until the production gates in `docs/product/production-readiness.md` and `docs/architecture/testing-and-verification.md` are truthfully satisfied.

## Superpowers workflow

Use Superpowers as engineering methodology and Beads as the work graph.

For architectural or creative changes:
1. brainstorming/design;
2. approved spec;
3. writing-plans;
4. execution.

For implementation:
- prefer `subagent-driven-development` when fresh subagents are supported;
- otherwise use `executing-plans`;
- use TDD for deterministic domain/game-state logic where feasible;
- use systematic debugging for failures;
- run verification before claiming completion.

Do not use Superpowers to create a parallel persistent task list outside Beads.

## Autonomous-run rules

The lead agent may make routine implementation choices that stay inside the authoritative docs and claimed issue.

Do NOT pause for:
- private/internal naming;
- minor code layout choices;
- reasonable UE5 API selection;
- test organization;
- small data-model choices;
- temporary placeholder substitution when the issue permits it;
- ordinary bug fixes required to complete the claimed issue.

Pause or record a blocker only when:
- required behavior is impossible in the environment;
- an external credential/license/asset is genuinely unavailable;
- requirements contradict each other;
- destructive repository operations are required;
- a production decision would materially change product scope or architecture.

If blocked, record exact evidence in Beads and continue with another independent ready issue.

## Human-only verification boundary

Automation may prove structure, state, buildability and deterministic behavior. It may **not** fabricate subjective player-experience evidence.

Human verification is required for claims about:
- mouse/camera feel;
- movement/vehicle feel;
- visual lighting correctness;
- presentation quality;
- readability;
- perceived hitching/pop-in;
- end-to-end game feel.

Do not repeatedly launch visible game windows while the user is working unless the issue explicitly requires an interactive run. Prefer bounded/offscreen automation. If evidence cannot be collected non-disruptively, record the exact limitation and create a human-playtest gate.

## UE5 architecture rules

- Unreal Engine 5 C++ project.
- Core gameplay/domain logic in C++.
- Blueprints remain thin presentation/configuration layers.
- Use Enhanced Input.
- Keep vehicle gameplay behind a facade so the current drivetrain can be replaced without rewriting game logic.
- Use `UGameInstanceSubsystem` / `UWorldSubsystem` only when lifecycle semantics justify them.
- Use Data Assets/Data Tables for content definitions where appropriate.
- Use versioned `USaveGame` persistence with explicit migration behavior.
- Do not put economy, contract resolution, needs, relationship, arrest, or save semantics into giant Blueprint graphs.
- Avoid plugins unless an issue requires them and they pass production/licensing review.
- Keep public interfaces narrow.
- Prefer components for actor-local behavior and services/subsystems for world/application state.

## Visual rules

The game is high-fidelity, not retro-low-poly.

Desired aesthetic:
- realistic ordinary city;
- late-2000s / early-2010s visual vocabulary;
- modern high-quality lighting/material presentation;
- mundane objects rendered beautifully;
- restrained dirty urban atmosphere.

Never drift into cyberpunk, synthwave, neon-hacker fantasy, VHS horror, PSX demake, or terminal-cliché aesthetics.

> Mundane objects should look expensive to render, not expensive to own.

Placeholder-flat geometry/materials are acceptable only while explicitly tracked as unfinished work. They are not a production-quality acceptance state.

## Performance is an architecture invariant

Read `docs/architecture/performance-budget.md` before rendering, world-building, asset, NPC, traffic or packaging work.

Hard rules:
- Low remains a supported art target.
- The baseline renderer must remain complete with Lumen disabled.
- Nanite/VSM/Lumen are optional quality layers, not baseline dependencies.
- No experimental renderer feature becomes a shipping dependency without measured evidence and explicit approval.
- Ordinary prop textures default to 512–1024; 2048 is the normal upper bound for important assets; 4096 requires justification; 8192 shipping content is prohibited.
- Package size and asset footprint are engineering budgets.
- Do not import marketplace/Megascans source-resolution content directly into shipping content.
- Avoid unrestricted Tick on ambient actors.
- Profile packaged builds; editor FPS is not production evidence.
- Renderer/city/asset issues are incomplete without their relevant budget checks.

The current generated District01 lighting may use movable components to avoid stale/unbuilt baked data. This is an **explicit production exception**, not permission for unlimited dynamic lighting. See `docs/decisions/active-exceptions.md` and keep the light count/shadow cost inside budget.

## Living-city invariant

Read `docs/architecture/city-simulation.md` before pedestrians, ambient NPC or traffic work.

Use hierarchical simulation:
- off-screen citizens: persistent data records;
- visible background population: lightweight agents/Mass where useful;
- nearby/important NPCs: full Character/Pawn behavior.

Do not build every citizen as an always-ticking `ACharacter`.

## Gameplay/interaction rules

The player remains vulnerable; avoid power-fantasy combat.

Prefer diegetic/physical action where reasonable:
- phone in hand;
- package physically picked up;
- vehicle interactions;
- food obtained in-world;
- bed used to sleep;
- delivery points physically interacted with.

Production hardening should improve feedback, responsiveness and legibility rather than replace gameplay with menus.

## Code quality

- Small focused files.
- Clear ownership of state.
- Data-driven definitions where useful.
- Deterministic domain logic where possible.
- No silent catch-all error handling.
- Avoid enormous managers owning unrelated systems.
- Buildable repository at completed checkpoints.
- Small atomic commits when the active agent profile permits commits.
- No credentials or secrets in tracked files.

## Completion protocol

Before saying an issue is complete:
1. run relevant tests;
2. run the correct UE build target when code changed;
3. run integration/commandlet/package checks required by the issue;
4. record any human-only verification still required;
5. inspect `git status --short`;
6. close/update Beads with exact evidence;
7. re-query `bd ready --json`.

Before ending a long autonomous run, follow `docs/agent/night-run.md`.
