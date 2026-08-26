# Courier 404 Pre-Production Vertical Slice Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Beads is authoritative for task state; use this plan as implementation guidance.

**Goal:** Build one cohesive UE5 pre-production vertical slice proving the ordinary-life delivery loop and optional anonymous high-risk loop.

**Architecture:** UE5 C++ owns deterministic gameplay/domain state, while Blueprints and levels provide composition and presentation. Systems communicate through explicit APIs/events and stable content identifiers. One compact district integrates all required systems.

**Tech Stack:** Unreal Engine 5, C++, Enhanced Input, Chaos Vehicles, UMG, Data Assets/Data Tables, USaveGame, Unreal Automation Tests, Git, Beads.

**Spec:** `docs/superpowers/specs/2026-08-25-courier404-preprod-design.md`

## Global Constraints

- Scope is fixed by `docs/product/preprod-scope.md`.
- High-fidelity modern UE5 rendering; no retro/PSX/VHS/cyberpunk pivot.
- Core gameplay semantics must be text-editable C++.
- Blueprints remain thin composition/presentation.
- One district and one car only.
- No large traffic/pedestrian ecosystem.
- Repository must remain buildable at completed checkpoints.
- Newly discovered required work goes into Beads.
- No fake verification.

---

## Execution mapping

The detailed atomic work is stored in Beads and seeded by `scripts/bootstrap_beads.sh`.

Use this plan to preserve sequence:

### Phase A — Foundation

- Bootstrap and verify UE5 C++ project.
- Establish build/test commands.
- Establish scalability/performance baseline and profiling harness before high-cost content.
- Add base game-state ownership, logging and interaction interfaces.
- Create compact district/apartment blockout with final lighting intent.
- Establish first-person controller and one drivable car.

### Phase B — Core ordinary life loop

- Contract data/runtime model.
- Physical package pickup/drop.
- Phone jobs UI.
- Normal delivery end-to-end.
- Economy.
- Clock/day-night.
- Hunger/fatigue.
- Food.
- Sleep.
- Save/load.

### Phase C — Human-life and risk layer

- Girlfriend planned interaction and scheduling consequence.
- Anonymous offer and high-payout contract.
- Police encounter with warning/fine/arrest.
- Hostile NPC encounter with flee/injury/robbery/death.
- Starvation/death integration.

### Phase D — Pre-prod integration

- Replace isolated demos with one cohesive slice.
- Establish art/lighting pass.
- Implement living-city hierarchical simulation baseline.
- Run low-end renderer/performance and cooked-size gate.
- Run main-loop verification.
- Fix P0/P1 blockers.
- Produce `docs/reports/preprod-status.md`.

## Interfaces to preserve

The exact class names may adapt to the existing repo, but do not collapse these conceptual boundaries:

- Contract definition vs runtime contract.
- Economy balance vs UI.
- Simulated clock vs wall clock.
- Needs state vs HUD.
- Relationship state vs messages.
- Police outcome semantics vs police animation.
- Hostile consequence semantics vs AI movement.
- Save payload vs world actors.
- Vehicle game-facing facade vs Chaos internals.

## Final verification

The final gate must prove:

```text
apartment
→ normal contract
→ car
→ pickup
→ delivery
→ payout
→ food
→ sleep
→ next period
→ anonymous offer
→ risky delivery
→ police and/or hostile consequence
→ relationship consequence
→ save/reload
```

Any missing required step remains an open Beads issue.


## Performance amendment

Before the final gate, Beads must contain and close work proving:
- renderer/scalability baseline;
- living-city hierarchical simulation;
- low-end packaged-build profiling;
- cooked package size.

Do not defer these to post-MVP polish.
