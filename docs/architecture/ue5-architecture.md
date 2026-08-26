# Courier 404 — UE5 Architecture

## Stack

- Unreal Engine 5
- C++ gameplay-first
- Blueprint as thin composition/presentation layer
- Enhanced Input
- Chaos Vehicles for the pre-prod car
- UMG for phone/HUD
- Data Assets/Data Tables for configurable content
- `USaveGame` for local persistence
- Automation tests for deterministic domain logic where practical
- Git + Beads + Superpowers
- hierarchical city simulation using Mass/StateTree/ZoneGraph/Smart Objects only where each tool reduces cost/complexity
- baked/static baseline renderer with optional scalable UE5 high-end features

## Module boundary

Prefer one game module for pre-prod unless a second module has a concrete build/test advantage.

Suggested namespace/module shape:

```text
Source/Courier404/
├── Core/
├── Interaction/
├── Player/
├── Vehicle/
├── Contracts/
├── Economy/
├── Needs/
├── Time/
├── Relationship/
├── Police/
├── Encounters/
├── Persistence/
└── UI/
```

Exact paths may follow existing repository conventions if a project already exists.

## State ownership

### Game-wide persistent state

Use a clearly scoped service/subsystem for:
- current money;
- current simulated time/day;
- relationship state;
- durable consequence flags.

Do not make a god-object that also controls actors, UI and mission scripting.

### Actor-local state

Components are preferred for:
- interactable behavior;
- inventory/cargo holder behavior;
- health/damage;
- needs if player-local;
- vehicle fuel/damage hooks.

### Contract state

Separate:
1. immutable contract definition;
2. runtime contract instance;
3. contract service/resolver;
4. presentation.

UI must not own contract truth.

## Data-driven contracts

Recommended conceptual types:

```cpp
USTRUCT(BlueprintType)
struct FContractDefinition
{
    FName ContractId;
    EContractCategory Category;
    FName PickupPointId;
    FName DropoffPointId;
    int32 Reward;
    float TimeLimitSeconds;
    FName CargoId;
    int32 RiskLevel;
    TArray<FName> Rules;
};
```

Exact UE types can change when implementation evidence demands it.

Runtime state should record:
- accepted;
- pickup complete;
- drop complete;
- failed/completed;
- relevant timestamps;
- dynamic destination if changed.

## Interaction

Use a reusable interaction interface/component.

Conceptual interface:

```cpp
CanInteract(Player)
GetInteractionPrompt(Player)
Interact(Player)
```

Do not hard-code every object type in the player character.

Candidates:
- door;
- bed;
- food item/store terminal;
- package;
- vehicle;
- locker;
- drop-off;
- phone-world interaction if needed.

## Player

Character responsibilities:
- locomotion;
- input routing;
- camera;
- interaction query;
- reference to actor-local components.

The character should not calculate contract payouts, arrests or relationship consequences.

## Vehicle

Use Chaos Vehicles unless existing repo constraints make it unsuitable.

Vehicle architecture should expose a small game-facing API:
- can enter;
- enter/exit;
- cargo placement;
- fuel query/consume if enabled;
- speed for traffic/police trigger logic;
- damage event.

Avoid coupling contract code to Chaos internals.

## Needs

Represent needs with deterministic values/state thresholds.

Need advancement should be testable without rendering.

Time advancement calls into needs.

Needs should emit state changes; presentation listens.

## Time

Central simulated clock with explicit advancement.

Do not derive all game state from wall-clock time.

Required operations:
- tick/advance simulation;
- sleep-to-time;
- query day/night;
- schedule availability windows.

## Relationship

Small persistent domain state.

No dialogue-framework dependency is required.

Suggested fields:
- strain/trust numeric or enum;
- planned interaction time;
- missed-plan flag;
- last interaction day.

Keep it small enough to replace later.

## Police

Police encounter should be state-driven.

Separate:
- trigger/observation;
- encounter state;
- outcome calculation;
- presentation/animation.

Pre-prod encounter may be heavily scripted spatially, but outcome semantics belong in C++.

## Hostile encounter

AI can use simple state machine / Behavior Tree if already standard in project.

Required states may be as small as:
- idle;
- warn;
- pursue;
- attack;
- disengage.

Do not overbuild.

## Persistence

Use explicit versioned save payload.

Conceptual:

```cpp
UCLASS()
class UCourier404SaveGame : public USaveGame
{
    int32 SaveVersion;
    int32 Money;
    float SimulatedTime;
    FNeedsSaveState Needs;
    FRelationshipSaveState Relationship;
    FContractSaveState ActiveContract;
    FPlayerResumeState Player;
};
```

On incompatible data:
- fail safely;
- log;
- use migration/default policy;
- never crash due to absent optional field.

## UI

UI reads from domain state and emits commands.

Do not let widgets directly mutate arbitrary world actors.

Phone screens:
- jobs;
- active job;
- anonymous offer;
- messages.

Keep pre-prod UI small.

## Error handling

Use:
- UE logging categories;
- ensure/check only where invariant semantics justify;
- explicit failure paths for recoverable gameplay errors;
- validation of Data Assets on load/use.

Do not silently continue with invalid pickup/drop identifiers.

## Performance

Pre-prod does not require broad optimization.

Avoid obvious hazards:
- expensive global actor scans every frame;
- Tick on every static interactable;
- unnecessary dynamic material creation;
- unbounded spawn loops.

Prefer events/timers where appropriate.

## Assets

Gameplay identifiers should not be raw scene-path assumptions when avoidable.

Use stable IDs/tags/data references so placeholder scenes can be replaced without rewriting domain logic.


## Renderer dependency rule

No domain/gameplay system may depend on Lumen, Nanite, VSM or another high-end rendering feature.

World actors must remain functionally valid when these features are disabled.

Use `docs/architecture/performance-budget.md` as the authoritative renderer/asset budget.

## Ambient population architecture

The city population is not represented as a single class of always-live Actor.

Maintain:
- persistent citizen records;
- lightweight ambient agents;
- full interactive NPC representation.

Promotion/demotion is governed by significance/visibility/gameplay importance.

Use `docs/architecture/city-simulation.md` as authoritative.
