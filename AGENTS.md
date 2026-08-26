# Courier 404 — Agent Operating Manual

## First action in every new agent context

Run:

```bash
bd prime
bd ready --json
```

Then read these files in order:

1. `docs/product/preprod-scope.md`
2. `docs/product/vision.md`
3. `docs/product/gameplay.md`
4. `docs/product/art-direction.md`
5. `docs/architecture/performance-budget.md`
6. `docs/architecture/city-simulation.md`
7. `docs/architecture/ue5-architecture.md`
8. `docs/architecture/testing-and-verification.md`
9. the Beads issue you are about to work on
10. the relevant Superpowers spec/plan for that issue

Do not begin feature implementation before this context is loaded.

## Source of truth

Beads is the authoritative task graph and long-horizon execution memory.

- Do not create Markdown TODO lists.
- Do not keep a second task tracker.
- Do not silently add scope.
- All newly discovered work must be represented in Beads.
- New bugs or required follow-up work discovered during an issue must use a `discovered-from` relationship when possible.
- Work only on issues that are ready unless resolving a blocker explicitly requires otherwise.

Typical workflow:

```bash
bd ready --json
bd show <issue-id> --json
bd update <issue-id> --claim --json
# implement and verify
bd close <issue-id> --reason "Completed and verified" --json
```

Dependency direction:

```bash
bd dep add <dependent-issue> <required-issue>
```

means the first issue cannot start until the second closes.

## Superpowers workflow

Use Superpowers as the engineering methodology and Beads as the work graph.

For architectural or creative changes:
1. brainstorming/design;
2. approved spec;
3. writing-plans;
4. execution.

For implementation:
- prefer `subagent-driven-development` when the harness supports fresh subagents;
- otherwise use `executing-plans`;
- use TDD for domain/game-state logic where feasible;
- use systematic debugging for failures;
- run verification before claiming completion.

Do not use Superpowers to create a parallel task list outside Beads. The plan describes implementation; Beads controls execution state.

## Overnight autonomy rules

The lead agent is authorized to make routine implementation choices that stay within the docs.

Do NOT pause for:
- naming of private/internal classes;
- minor layout choices;
- reasonable UE5 API selection;
- test organization;
- small data-model choices;
- placeholder asset substitution;
- ordinary bug fixes.

Do pause/record a blocker only when:
- the requested behavior is impossible with the available environment;
- a required external license/credential is unavailable;
- the project would require violating the fixed scope or architecture;
- destructive repository changes are needed;
- there are contradictory authoritative requirements.

If blocked, create or update a Beads issue with exact evidence and continue with another independent ready issue.

## Product scope

The overnight goal is the **pre-production vertical slice** in `docs/product/preprod-scope.md`.

The goal is NOT:
- a large open world;
- a complete commercial game;
- a GTA clone;
- a full life simulator;
- a full dating simulator;
- a full police simulation;
- a combat game;
- a hacking simulator;
- a procedural city.

YAGNI aggressively.

## UE5 architecture rules

- Unreal Engine 5 C++ project.
- Core gameplay/domain logic in C++.
- Blueprints are a thin presentation/configuration layer.
- Use Enhanced Input.
- Prefer Chaos Vehicles for the MVP car.
- Use `UGameInstanceSubsystem` / `UWorldSubsystem` only when lifecycle semantics justify them.
- Use Data Assets/Data Tables for content definitions.
- Use `USaveGame` for persistence.
- Do not put economy, contract resolution, needs-state semantics, relationship state, arrest state or save semantics into giant Blueprint graphs.
- Avoid adding plugins unless the issue explicitly requires them.
- Keep public interfaces narrow.
- Prefer components for actor-local behavior and services/subsystems for world/application state.

## Visual rules

The game is high-fidelity, not retro-low-poly.

The desired aesthetic is:
- realistic ordinary city;
- late-2000s / early-2010s visual vocabulary;
- high-quality modern UE5 lighting and materials;
- mundane objects rendered beautifully;
- restrained dirty urban atmosphere.

Never turn the project into:
- cyberpunk;
- synthwave;
- neon hacker fantasy;
- exaggerated VHS horror;
- PSX demake;
- green-terminal cliché.

Art rule:

> Mundane objects should look expensive to render, not expensive to own.

## Performance is an architecture invariant

Read `docs/architecture/performance-budget.md` before touching rendering, world building, assets, NPCs, traffic or packaging.

Hard rules:

- The baseline renderer must remain complete with Lumen disabled.
- Nanite may be used selectively but may not replace asset/LOD discipline.
- VSM is a scalable High/Ultra option, not a baseline dependency.
- MegaLights/experimental renderer features may not become shipping dependencies without a measured profiling issue and explicit approval.
- Default ordinary prop textures are 512–1024; 2048 is the normal upper bound for important assets; 4096 requires justification; 8192 shipping content is prohibited.
- Low scalability is a supported art target.
- The project must be profiled in packaged builds, not judged by editor FPS alone.
- Asset size and package size are engineering budgets.
- Do not import and ship marketplace/Megascans assets at source resolution without reduction, LOD/fallback and size review.
- Do not add unrestricted Tick to ambient actors.
- No issue that materially changes rendering, city population or asset footprint is complete without checking its relevant performance budget.

Reference targets and exact frame/size budgets live in `docs/architecture/performance-budget.md`.

## Living city invariant

Read `docs/architecture/city-simulation.md` before implementing pedestrians, ambient NPCs or traffic.

The city must feel alive, but full AI is reserved for nearby/important characters.

Use hierarchical simulation:

- off-screen citizens: persistent data records;
- visible background population: lightweight agents/Mass where useful;
- nearby/important NPCs: full Character/Pawn behavior.

Do not build every citizen as an always-ticking `ACharacter`.

## Gameplay rules

The player is vulnerable.

Allowed consequences in the slice:
- hunger penalties;
- eventual starvation from prolonged neglect;
- exhaustion penalties;
- injury;
- robbery after losing a fight;
- failed delivery;
- police stop;
- fine;
- detention/arrest;
- death.

Do not build power-fantasy combat.

The player may shove, strike, block and flee if the current issue requires it. Against multiple hostile NPCs, fleeing should generally be the rational option.

## Relationship rules

The girlfriend is a person with scheduled availability, not a meter dispenser.

The pre-prod slice needs:
- one planned interaction/date or at-home interaction;
- one relationship consequence caused by the player's scheduling choice;
- persistence of relationship state.

Do not implement a branching romance campaign.

## Interaction rules

Prefer diegetic/physical action:
- phone in hand;
- package physically picked up;
- car doors/trunk usable;
- food obtained in-world;
- bed used to sleep;
- delivery point physically interacted with.

Avoid replacing gameplay with menu buttons where physical interaction is reasonable.

## Code quality

- Small focused files.
- Clear ownership of state.
- Data-driven contract definitions.
- Deterministic domain logic where possible.
- No silent catch-all error handling.
- Avoid enormous managers that own unrelated systems.
- Buildable repository after every completed issue.
- Small atomic commits.

## Verification gate

Never close a Beads issue only because code was written.

Verify according to `docs/architecture/testing-and-verification.md`.

If a build/test cannot run because the environment lacks UE5, say so explicitly in the issue and commit message. Do not fabricate success.
