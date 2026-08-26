# Courier 404 — Production Readiness Contract

## Status

This document defines the active product-development phase after the completed pre-production vertical slice.

The vertical slice proved that the systems can exist together. The production phase must make the game **feel intentional, look coherent, remain reliable, scale predictably and ship reproducibly**.

Beads owns current task state. This document owns the production acceptance boundary.

## Product priority order

When priorities conflict, prefer:

1. player feel and correctness;
2. crash/data-loss prevention;
3. UX/readability;
4. stable performance on target hardware;
5. coherent presentation and content quality;
6. systemic/living-world depth;
7. new feature breadth.

Do not add breadth to hide weak fundamentals.

## Production pillars

### 1. Player feel

The first-person controller must feel deliberate:
- consistent mouse input;
- configurable sensitivity;
- no accidental acceleration/smoothing unless intentionally designed;
- sane FOV and camera behavior;
- movement acceleration/deceleration tuned for grounded first-person play;
- interaction response is immediate and legible;
- vehicle handling has predictable input, camera and feedback.

Human playtest acceptance is mandatory.

### 2. Presentation

The world must stop reading as a blockout:
- coherent material family;
- deliberate lighting/exposure across time-of-day phases;
- readable interiors/exteriors;
- recognizable prop/environment language;
- restrained atmosphere consistent with art direction;
- interaction/audio/animation feedback at important actions;
- placeholders explicitly tracked and burned down.

Automated light/asset audits supplement but do not replace visual review.

### 3. Living city

The city should feel occupied without simulating everyone at full fidelity.

Production work should improve:
- activity-anchored NPC placement;
- zone-specific density and routines;
- visible ambient behaviors;
- time-of-day thinning;
- reactions to nearby disturbances;
- traffic/courier/worker ambience where affordable.

The hierarchical simulation contract remains mandatory.

### 4. Reliability and persistence

Required production behaviors:
- versioned save format;
- explicit migration path for compatible versions;
- safe handling of invalid/corrupt saves;
- no duplicate economic rewards after reload;
- deterministic restoration of authoritative state;
- stable recovery after death/arrest/failure;
- no main-loop softlocks in known paths;
- actionable logs for serious failures.

### 5. Performance and scalability

Production truth comes from packaged builds.

Required:
- Low remains visually complete;
- target budgets in `performance-budget.md` are measured, not assumed;
- representative traversal has bounded hitching;
- texture/streaming budgets are enforced;
- visible city density scales with preset without deleting critical actors;
- PSO/shader startup hitches are measured and reduced where material;
- no visible-window profiling loops that disrupt the user without explicit need.

### 6. Shipping pipeline

A production candidate requires:
- Development and Shipping targets build reproducibly;
- package from a clean state;
- clean-machine launch protocol documented;
- required runtime files/assets included;
- no source-only or editor-only dependency on the playable path;
- no secrets/API keys in tracked files or shipped package;
- third-party/plugin/license inventory understood;
- release verification report generated from evidence.

## Production gate ladder

### Gate A — Playable-quality foundation

Must pass before large content expansion:
- input/mouse/movement feel accepted by human playtest;
- vehicle basic feel accepted;
- interaction feedback readable;
- startup lighting visually sane on real renderer;
- placeholder-flat presentation has an explicit replacement path;
- existing core loop remains functional.

### Gate B — Alpha-quality systems

- no known P0 main-loop defects;
- save/load/recovery reliable;
- main gameplay systems have automated coverage;
- city density and encounters remain stable during representative play;
- settings needed for regular playtesting exist;
- packaged build is the default playtest artifact.

### Gate C — Performance/content production

- Low and primary target presets measured;
- representative traversal meets or has tracked blockers against budgets;
- asset-size/texture/LOD discipline enforced;
- production material/prop/environment language established;
- recurring startup/runtime hitch sources have owners/issues.

### Gate D — Release-candidate readiness

Do not claim this gate from automation alone.

Required:
- Shipping build from clean checkout/worktree;
- clean-machine launch test;
- full main-loop human playtest;
- save/load across representative progression;
- no open P0; P1 release blockers explicitly resolved/waived by owner;
- performance evidence on reference hardware;
- crash/error diagnostics reviewed;
- package/content/license/security audit;
- release report records exact known limitations.

## Scope discipline

Production readiness is not permission to build every imagined feature.

Before adding a new system ask:
- Does it improve the core courier/life/risk fantasy?
- Is a current production gate blocked without it?
- Is there a Beads issue with acceptance criteria?
- Can the existing architecture absorb it without destabilizing proven systems?

If not, backlog it.

## Current known human findings

The first real packaged playtest established at least these production-quality concerns:
- mouse/movement feel needs tuning;
- the world reads as visibly placeholder/bare;
- lighting correctness must be judged visually, not only by structural audits;
- performance capture tooling should be bounded and non-disruptive.

These are findings, not a replacement task tracker. Represent actionable work in Beads.
