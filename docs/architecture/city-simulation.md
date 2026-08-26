# Courier 404 — Living City Simulation Architecture

## Goal

The city must feel as though it continues living independently of the player while remaining inexpensive enough for the project's low-end performance targets.

Do not interpret "living city" as "full AI simulation for every NPC."

The implementation goal is **perceptual continuity with hierarchical simulation**.

## Core rule

> Simulate what the player can meaningfully observe. Abstract everything else.

A citizen can exist at three fidelity levels.

## Tier 0 — Persistent record

Used when the citizen is not meaningfully observable.

Representation is data only.

Conceptual state:

```text
CitizenId
Archetype
HomeZone
WorkZone
CurrentZone
ScheduleState
Activity
RelationshipFlags
LastSimulatedTime
PersistentEventFlags
```

No:
- Actor;
- skeletal mesh;
- animation;
- pathfinding;
- per-frame Tick.

Target capacity for the pre-production architecture:
- at least 250 logical citizen records without meaningful frame-time cost.

Simulation advances in coarse time steps.

## Tier 1 — Lightweight ambient agent

Used for background population that the player can see but is not interacting with.

Preferred UE5 direction:
- Mass Entity where it reduces actor/component overhead;
- ZoneGraph for movement corridors when justified;
- cheap scheduled updates;
- simplified animation representation;
- no expensive perception system.

Pre-production target:
- up to ~40–60 lightweight ambient agents in the active district under the representative Medium/High profile;
- density scales down on Low.

This is a ceiling to profile against, not a requirement to fill every street.

## Tier 2 — Full interactive NPC

Used for:
- girlfriend;
- police currently interacting with player;
- hostile encounter NPCs;
- nearby citizens whose reactions are visible;
- any NPC directly participating in a gameplay event.

Preferred representation:
- Character/Pawn;
- StateTree or small explicit state machine;
- Navigation only while needed;
- reaction/perception bounded by distance and relevance;
- animation update budget enabled.

Initial target:
- approximately 8–16 fully simulated NPCs in the player's immediate area;
- avoid designing scenarios that require dozens of full Characters simultaneously.

## Promotion/demotion

Citizens move between fidelity tiers based on:

- distance;
- visibility;
- current activity;
- narrative/gameplay importance;
- active interaction;
- event participation.

Promotion should reconstruct a plausible local state from the persistent record.

Demotion should write meaningful persistent state back before destroying/recycling expensive representation.

Do not require exact centimeter-perfect off-screen simulation.

## Schedule model

Citizens should have simple schedules that produce believable population patterns.

Example activities:

- home;
- commute;
- work;
- shop;
- wait;
- socialize;
- smoke;
- walk;
- drive;
- sleep.

A schedule entry identifies:
- time window;
- zone;
- activity;
- optional Smart Object/category preference.

No general-purpose GOAP planner is required.

## Time-of-day population

Population density and archetypes must change by time.

Examples:

Morning:
- commuters;
- delivery workers;
- opening stores.

Day:
- shoppers;
- workers;
- couriers;
- service activity.

Evening:
- returning commuters;
- social groups;
- denser traffic around shops.

Night:
- reduced ordinary foot traffic;
- taxis;
- late couriers;
- convenience-store activity;
- police;
- small social groups in courtyards.

The night must feel different without becoming an empty horror map.

## Ambient activities

Use Smart Objects or equivalent reusable activity anchors for:

- bench sitting;
- smoking;
- phone checking;
- vending/ATM use;
- shopping;
- waiting at crossing/bus stop;
- talking in small groups;
- loading/unloading a vehicle.

One reusable activity system is better than bespoke scripting for every scene.

## Ambient events

Small events create disproportionate life.

Pre-production should support at least three reusable event archetypes, for example:

- minor traffic incident;
- people arguing;
- courier unloading;
- police roadside stop;
- person struggling with a car;
- small group socializing.

Events should be data/configuration-driven where practical.

They do not need quest logic.

## Persistence illusion

The city should remember selected events cheaply.

Use event flags/timestamps such as:

```text
CourtyardFightRecent
IntersectionCrashResolved
StoreClosedForNight
PolicePresenceUntil
```

The next visit may select a different scene variant.

Do not persist every dropped can or every pedestrian path.

## Reactions

Nearby full NPCs should respond to major visible events:

- collision;
- fight;
- police activity;
- player sprinting into them;
- severe disturbance.

Reactions can be simple:
- look;
- step away;
- flee;
- call/alert;
- stop current activity.

No human-level social AI is required.

## Traffic

Traffic is hierarchical too.

Pre-production goals:

- only nearby moving vehicles require full collision/driver behavior;
- parked vehicles supply most visual density cheaply;
- distant movement can be simplified or represented by lightweight agents;
- spawn/despawn/recycle outside meaningful observation;
- density varies by time.

Do not build a GTA-scale traffic simulation.

A compact district can feel busy with:
- many parked cars;
- a small number of nearby moving cars;
- distant audio/lights;
- carefully timed crossings.

## Buildings and windows

Do not simulate inaccessible apartments.

Cheap life cues:
- window emissive variation;
- occasional silhouette/curtain animation;
- television light flicker;
- interior audio zones;
- timed lights.

These cues must use inexpensive materials/instances and bounded update rates.

## Update discipline

No ambient system gets unrestricted per-frame Tick by default.

Prefer:
- timers;
- event-driven updates;
- Mass processors;
- significance-based update frequency;
- Animation Budget Allocator / update-rate optimization where appropriate.

Full NPC update rate may fall with distance when visually acceptable.

## Determinism and testing

Schedule and tier-transition logic should be separable from rendering.

Tests should cover:
- schedule state selection;
- promotion/demotion state preservation;
- time-of-day density profile selection;
- event persistence flags;
- safe despawn during no active interaction.

## Scalability

Population scales independently from core gameplay.

Low:
- fewer lightweight agents;
- fewer full ambient NPCs;
- shorter active distances;
- lower animation update rate;
- fewer moving ambient vehicles.

Medium/High:
- progressively higher density.

Critical gameplay NPCs and encounter semantics must not disappear because crowd quality is Low.

## Pre-production acceptance

The district is considered "alive enough" for the vertical slice when:

1. population visibly changes between at least day/evening/night states;
2. at least three ambient activities are observable;
3. at least one ambient event can occur without being a mission;
4. nearby civilians react to one major disturbance;
5. the system survives repeated spawn/promotion/demotion without leaks or stuck gameplay;
6. Low scalability remains functional;
7. the city does not depend on full AI for off-screen citizens.
