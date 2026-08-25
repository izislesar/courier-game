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
5. `docs/architecture/ue5-architecture.md`
6. `docs/architecture/testing-and-verification.md`
7. the Beads issue you are about to work on
8. the relevant Superpowers spec/plan for that issue

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

<!-- BEGIN BEADS INTEGRATION v:1 profile:minimal hash:970c3bf2 -->
## Beads Issue Tracker

This project uses **bd (beads)** for issue tracking. Run `bd prime` to see full workflow context and commands.

### Quick Reference

```bash
bd ready              # Find available work
bd show <id>          # View issue details
bd update <id> --claim  # Claim work
bd close <id>         # Complete work
```

### Rules

- Use `bd` for ALL task tracking — do NOT use TodoWrite, TaskCreate, or markdown TODO lists
- Run `bd prime` for detailed command reference and session close protocol
- Use `bd remember` for persistent knowledge — do NOT use MEMORY.md files

**Architecture in one line:** issues live in a local Dolt DB; sync uses `refs/dolt/data` on your git remote; `.beads/issues.jsonl` is a passive export. See https://github.com/gastownhall/beads/blob/main/docs/SYNC_CONCEPTS.md for details and anti-patterns.

## Agent Context Profiles

The managed Beads block is task-tracking guidance, not permission to override repository, user, or orchestrator instructions.

- **Conservative (default)**: Use `bd` for task tracking. Do not run git commits, git pushes, or Dolt remote sync unless explicitly asked. At handoff, report changed files, validation, and suggested next commands.
- **Minimal**: Keep tool instruction files as pointers to `bd prime`; use the same conservative git policy unless active instructions say otherwise.
- **Team-maintainer**: Only when the repository explicitly opts in, agents may close beads, run quality gates, commit, and push as part of session close. A current "do not commit" or "do not push" instruction still wins.

## Session Completion

This protocol applies when ending a Beads implementation workflow. It is subordinate to explicit user, repository, and orchestrator instructions.

1. **File issues for remaining work** - Create beads for anything that needs follow-up
2. **Run quality gates** (if code changed) - Tests, linters, builds
3. **Update issue status** - Close finished work, update in-progress items
4. **Handle git/sync by active profile**:
   ```bash
   # Conservative/minimal/default: report status and proposed commands; wait for approval.
   git status

   # Team-maintainer opt-in only, unless current instructions forbid it:
   git pull --rebase
   bd dolt push
   git push
   git status
   ```
5. **Hand off** - Summarize changes, validation, issue status, and any blocked sync/commit/push step

**Critical rules:**
- Explicit user or orchestrator instructions override this Beads block.
- Do not commit or push without clear authority from the active profile or the current user request.
- If a required sync or push is blocked, stop and report the exact command and error.
<!-- END BEADS INTEGRATION -->

<!-- BEGIN BEADS CODEX SETUP: generated by bd setup codex -->
## Beads Issue Tracker

Use Beads (`bd`) for durable task tracking in repositories that include it. Use the `beads` skill at `.agents/skills/beads/SKILL.md` (project install) or `~/.agents/skills/beads/SKILL.md` (global install) for Beads workflow guidance, then use the `bd` CLI for issue operations.

### Quick Reference

```bash
bd ready                # Find available work
bd show <id>            # View issue details
bd update <id> --claim  # Claim work
bd close <id>           # Complete work
bd prime                # Refresh Beads context
```

### Rules

- Use `bd` for all task tracking; do not create markdown TODO lists.
- Run `bd prime` when Beads context is missing or stale. Codex 0.129.0+ can load Beads context automatically through native hooks; use `/hooks` to inspect or toggle them.
- Keep persistent project memory in Beads via `bd remember`; do not create ad hoc memory files.

**Architecture in one line:** issues live in a local Dolt DB; sync uses `refs/dolt/data` on your git remote; `.beads/issues.jsonl` is a passive export. See https://github.com/gastownhall/beads/blob/main/docs/SYNC_CONCEPTS.md for details and anti-patterns.
<!-- END BEADS CODEX SETUP -->
