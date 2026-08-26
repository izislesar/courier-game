# Courier 404 Pre-Production Vertical Slice — Design Baseline

## Status

Product baseline approved by conversation context.

This document is the starting architectural design for the overnight pre-production slice. Agents may refine implementation detail, but must not broaden scope beyond `docs/product/preprod-scope.md`.

## Product shape

A first-person immersive urban delivery/life simulation in a compact district.

The player has:
- an apartment/home base;
- a single used car;
- a phone;
- ordinary delivery work;
- a relationship obligation;
- food/sleep needs;
- access to an optional anonymous delivery;
- exposure to mundane police enforcement and street violence.

The slice demonstrates the contrast between ordinary life and the hidden high-risk economy.

## Architecture

Use UE5 C++ for core state and rules.

Gameplay is split into bounded systems:
- interaction;
- player state;
- vehicle facade;
- contracts;
- time;
- needs;
- economy;
- relationship;
- police outcomes;
- hostile encounter;
- persistence.

Presentation binds these systems into one district map.

## Primary data flow

```text
Phone accepts ContractDefinition
→ ContractService creates RuntimeContract
→ World pickup actor binds by stable pickup ID
→ Interaction marks pickup complete
→ Cargo becomes carried/vehicle cargo
→ Drop actor validates active runtime contract
→ ContractService resolves completion
→ Economy applies payout
→ Relationship/police consequence hooks consume events
→ Persistence serializes durable state
```

## Time/needs flow

```text
SimulationClock advances
→ NeedsService consumes elapsed simulated time
→ hunger/fatigue thresholds change
→ player component receives mechanical modifiers
→ presentation updates feedback
```

Sleep performs an explicit time jump and recovery operation.

## Relationship flow

```text
planned interaction scheduled
→ player spends time elsewhere / accepts conflicting contract
→ clock passes deadline
→ relationship resolver records missed plan
→ message/presentation reflects consequence
→ save persists result
```

## Police flow

```text
world trigger / driving trigger
→ police encounter state
→ outcome resolver evaluates configured context
→ warning OR fine OR detention
→ contract/relationship/economy consequences
→ return player to stable world state
```

## Hostile flow

```text
proximity/context trigger
→ warn
→ pursue/attack
→ player flees OR loses OR wins/disengages
→ consequence resolver
→ injury/robbery/death
```

Combat depth is intentionally minimal.

## World composition

One map/district should be enough.

If asset production is constrained:
- block out first;
- establish final lighting intent early;
- use replaceable actor Blueprints and Data Assets;
- never entangle gameplay code with final meshes.

## Testing

Domain systems must be testable independently from presentation.

Integration gates prove the actual playable loop.

## Scope enforcement

Any attractive feature not required by `preprod-scope.md` becomes a Beads backlog issue and is not implemented during the overnight execution.


## Renderer/scalability architecture amendment

The vertical slice uses a raster/baked-light baseline. High-end UE5 technologies are additive scalability features.

The architecture must prove:
- Low works without Lumen;
- Nanite is optional and fallback-safe;
- VSM is not required;
- asset budgets are measured;
- packaged build size is measured;
- city simulation uses hierarchical fidelity.

Authoritative details:
- `docs/architecture/performance-budget.md`
- `docs/architecture/city-simulation.md`
