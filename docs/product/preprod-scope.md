# Courier 404 — Pre-Production Vertical Slice Scope

## Goal

Produce one cohesive, playable, saveable vertical slice that proves the product fantasy and engineering architecture.

This is the overnight target.

## Definition of "done"

A new player can launch the game and complete this path without editor intervention:

1. Spawn/wake in the apartment.
2. Observe current money, time and basic status through diegetic/minimal UI.
3. Use the phone to accept one normal delivery.
4. Leave the apartment and reach the vehicle.
5. Enter and drive the vehicle.
6. Reach a pickup point.
7. Physically collect a package.
8. Put/carry the package appropriately.
9. Drive to the normal drop-off.
10. Deliver successfully and receive payment.
11. Buy/eat food or otherwise satisfy hunger.
12. Return home and sleep.
13. Advance to another day/night period.
14. Receive at least one anonymous contract.
15. Choose whether to accept it.
16. If accepted, perform pickup and risky drop.
17. Encounter at least one police-stop path.
18. Encounter at least one hostile-NPC path.
19. Be capable of injury, arrest/detention and death.
20. Save and reload persistent state.

The slice must have a clear start and a stable end-state.

## Required playable area

Exactly one compact district is required.

It must contain or convincingly proxy:

- player apartment;
- street/parking area;
- convenience store or food source;
- one ordinary pickup location;
- one ordinary drop-off;
- one anonymous pickup locker/drop;
- one risky destination;
- one police encounter zone/path;
- one hostile encounter location;
- enough ambient population/traffic representation to demonstrate the living-city architecture.

No second district is required.

## Required systems

### Player

- first-person movement;
- interaction trace;
- carry/drop package interaction;
- basic stamina;
- health/injury state;
- minimal shove/hit/block/flee support only if required for hostile encounter.

### Vehicle

One drivable ordinary used car.

Required:
- enter/exit;
- steering;
- accelerate/brake/reverse;
- basic collision consequence;
- headlights;
- cargo/trunk interaction or equivalent cargo placement;
- fuel state if it can be implemented without destabilizing the slice.

No vehicle purchasing system is required.

### Phone

Minimal diegetic phone with:
- normal delivery job;
- anonymous offer;
- current job/objective;
- messages relevant to girlfriend and contract consequences.

Do not build a general smartphone OS.

### Contracts

At least:
- one normal contract definition;
- one anonymous contract definition;
- one modifier or rule demonstrating data-driven extensibility.

Contract resolution must be deterministic and testable outside presentation logic where feasible.

### Money/economy

- money balance;
- delivery payout;
- food purchase;
- fine and/or medical/incident cost;
- persistence.

No complete shop economy.

### Needs

Required states:

Hunger:
- Fed
- Hungry
- Very Hungry
- Starving

Sleep/fatigue:
- Rested
- Tired
- Exhausted

Needs should progress slowly enough that one missed meal or one late night is not catastrophic.

Starvation must require prolonged neglect over multiple in-game cycles.

### Time

- game clock;
- day/night transition;
- sleep advances time;
- certain contract availability can depend on time.

### Relationship

Minimum viable relationship loop:
- one girlfriend contact;
- one scheduled/planned interaction;
- one scheduling conflict with work;
- one persistent consequence.

Do not build a dialogue tree framework beyond what the slice needs.

### Police

One systemic encounter prototype.

Possible triggers:
- traffic violation;
- suspicious location;
- contract-specific trigger.

Possible outcomes:
- warning;
- fine;
- detention/arrest.

Arrest does not have to be game over.

A minimal arrest result may:
- advance time;
- fail active contract;
- apply monetary loss/fine;
- alter relationship state if applicable;
- return player to a known location.

### Hostile NPC

One encounter archetype:
- one or more aggressive street NPCs;
- threat should be avoidable when possible;
- player can flee;
- losing may cause injury/robbery;
- death is possible in severe outcome.

No sophisticated melee combat tree.

### Death

Required death sources in architecture:
- violence;
- severe collision or generic lethal damage;
- starvation after prolonged neglect.

Pre-prod only needs at least one reliably testable playable death path plus data/model support for the others.

### Living city baseline

The vertical slice must prove the hierarchical city-simulation approach without turning the slice into a population simulator.

Required:
- at least day/evening/night population-density variation;
- at least three ambient activities;
- at least one non-mission ambient event;
- nearby civilian reaction to one major disturbance;
- scalable ambient population that can be reduced on Low without removing critical gameplay NPCs.

Off-screen citizens must not require live Character actors.

### Performance/scalability

The slice must comply with `docs/architecture/performance-budget.md`.

Required:
- a complete Low renderer path with Lumen disabled;
- no required Nanite/VSM dependency;
- Low/Medium/High scalability configuration;
- representative packaged-build profiling;
- texture/material discipline;
- package-size measurement;
- performance status written to `docs/reports/performance/`.

The visual direction must survive the Low preset.

### Save/load

Persist at minimum:
- money;
- time/day;
- needs;
- player state needed for continuity;
- relationship state;
- completed/active contract state where safe;
- position or safe resume location;
- relevant police/consequence state.

## Visual target

The slice should use modern UE5 rendering and establish final-quality lighting intent.

Required:
- Lumen-ready lighting setup or equivalent UE5 high-fidelity real-time GI path;
- coherent exposure;
- strong night lighting;
- believable apartment lighting;
- believable street lighting;
- fog/atmosphere only when tasteful;
- material consistency;
- no retro filter.

Temporary meshes are allowed where licensed final assets are not available, but placeholder geometry must not justify placeholder architecture.

## Pre-prod quality bar

A pre-prod candidate is acceptable when:
- the complete gameplay loop is playable;
- systems are integrated rather than isolated demos;
- core state persists;
- code architecture is extensible;
- repeated play does not immediately break state;
- the project builds cleanly in the available environment;
- the representative Low path satisfies the project's current performance gate or has exact measured blockers recorded;
- package size is measured and within the pre-production review threshold;
- critical/high-severity known bugs are recorded in Beads;
- no fake completion claims remain.

## Explicitly out of scope overnight

- second district;
- traffic ecosystem;
- full pedestrian simulation;
- crowds;
- multiple drivable vehicles;
- motorcycles;
- vehicle ownership/progression;
- procedural city generation;
- detailed inventory grid;
- crafting;
- skill tree;
- weapons;
- firearms;
- police chase AI;
- full wanted-level system;
- full melee combat;
- multiple girlfriends/romance routes;
- sexual content;
- branching cinematic story;
- large quest chain;
- multiplayer;
- online services;
- backend;
- accounts;
- telemetry service;
- workshop/mod support;
- Steam integration;
- console support.

## Scope-change rule

Any feature not explicitly required above defaults to OUT OF SCOPE until added as a Beads backlog issue.

Do not implement backlog work during the overnight run.
